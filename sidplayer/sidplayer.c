#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "6510.h"
#include "console-interface.h"

#include "platform-support-rpi.h"
#include "platform-support-bbb.h"
#include "platform-support-dummy.h"

FILE *inputSidFile = NULL;
struct reveller_platform *reveller = NULL;

void detect_platform() {
    reveller = &dummy_platform;

    struct stat info;
    if (stat("/proc", &info) != 0) {
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

void printWelcome() {
    printf("Reveller SID, running on %s\n", reveller->platform_id);
    c64_printOpcodeStats();
}

int main(int argc, char **argv) {
    detect_platform();
    reveller->init();

	printWelcome();
	
	if (argv[1]) {
		inputSidFile = fopen(argv[1], "rb");
		if (inputSidFile == 0) {
			printf("ERROR: File %s not found\n", argv[1]);
			exit(1);
		}

		parseHeader();
		printf("Loaded %s, %d subsongs\n", argv[1], sh.songs);
	}

	console_interface();
	return 0;
}
