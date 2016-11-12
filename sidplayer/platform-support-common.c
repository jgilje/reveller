#include "platform-support-common.h"
#include "platform-support.h"

#include "c64emu/6510.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <sched.h>

FILE* reveller_input_file = NULL;

void common_platform_debug(const char *msg, ...) {
    va_list argp;
    
    va_start(argp, msg);
    vfprintf(stdout, msg, argp);
    va_end(argp);
    
    fflush(stdout);
}

void common_platform_abort(const char *msg, ...) {
    va_list argp;
    
    va_start(argp, msg);
    vfprintf(stderr, msg, argp);
    va_end(argp);
    
    fflush(stdout);
    fflush(stderr);
    
    exit(1);
}

size_t common_platform_read_source(uint32_t offset, uint32_t length, uint8_t *dest) {
    size_t ret;
    // printf("Reading at offset %d - %d bytes\n", offset, length);
    if (fseek(reveller_input_file, offset, SEEK_SET) < 0) {
        common_platform_abort("Failed to seek inputSidFile, offset %x\n", offset);
    }
    ret = fread(dest, 1, length, reveller_input_file);
    return ret;
}

void common_sid_block_start() {
}

void common_sid_block_end() {
}

void common_pause() {
    reveller->sid_write(0x18, 0);
}
void common_resume() {
    reveller->sid_write(0x18, c64_sid_register[0x18]);
}

static int fd_mem = -1;
int open_mem() {
    if ((fd_mem = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        printf("Could not get /dev/mem\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void* get_addr(uint32_t addr) {
    void* m = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, addr & ~MAP_MASK);
    if(m == (void *) -1) {
        printf("Failed to map addr.: %x\n", addr);
        exit(-1);
    }

    return m;
}

void release_addr(void* addr) {
    int r = munmap(addr, MAP_SIZE);
    if (r != 0) {
        printf("Failed to unmap addr.: %p\n", addr);
        exit(-1);
    }
}

void set_realtime() {
    struct sched_param param;
    param.sched_priority = 20;
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        printf("Failed to get RealTime priority");
    }
}

extern struct reveller_platform stream_platform;
extern struct reveller_platform dummy_platform;
extern struct reveller_platform rpi_platform;
extern struct reveller_platform rpi2_platform;
extern struct reveller_platform bbb_platform;
struct reveller_platform *reveller = NULL;

void detect_platform() {
    reveller = &dummy_platform;

    struct stat info;
    if (stat("/dev/reveller", &info) == 0) {
        reveller = &stream_platform;
        return;
    } else if (stat("/proc", &info) != 0) {
        return;
    } else if (info.st_mode & S_IFDIR) {    // S_ISDIR() if avail. on windows
        FILE *fp;

        if ((fp = fopen("/proc/cpuinfo", "r")) == NULL) {
            return;
        }

        char buffer[1024];
        char hardware[1024];
        while(! feof(fp)) {
            fgets(buffer, sizeof(buffer), fp);
            int res = sscanf(buffer, "Hardware	: %1023c", hardware);
            hardware[1023] = 0;

            switch (res) {
            case 0:
                break;
            case EOF:
                break;
            default:
                if (strstr(hardware, "BCM2708") != NULL) {
                    reveller = &rpi_platform;
                } else if (strstr(hardware, "BCM2709") != NULL) {
                    reveller = &rpi2_platform;
                } else if (strstr(hardware, "AM33XX") != NULL) {
                    reveller = &bbb_platform;
                }

                break;
            }
        }

        fclose(fp);
    } else {
        return;
    }
}
