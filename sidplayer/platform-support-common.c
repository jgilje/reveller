#include "platform-support-common.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/mman.h>

#include <sched.h>

extern FILE* inputSidFile;
extern FILE* outfile;

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
    if (fseek(inputSidFile, offset, SEEK_SET) < 0) {
        common_platform_abort("Failed to seek inputSidFile, offset %x\n", offset);
    }
    ret = fread(dest, 1, length, inputSidFile);
    return ret;
}

void common_sid_block_start() {
}

void common_sid_block_end() {
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
