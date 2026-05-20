#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <sys/types.h>

#include "6510.h"
#include "console-interface.h"

#include "platform-support-common.h"
#include "platform-support.h"

void printWelcome() {
    printf("Reveller SID, running on %s\n", reveller->platform_id);
    c64_printOpcodeStats();
}

int main(int argc, char **argv) {
    int last = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--force-pdsid") == 0) {
            sidchip_implementation = SIDCHIP_PDSID;
        } else {
            last = i;
        }
    }

    detect_platform();
    detect_chip();
    reveller->init();

	printWelcome();

	if (last > 0 && argv[last]) {
        reveller_input_file = fopen(argv[last], "rb");
        if (reveller_input_file == 0) {
			printf("ERROR: File %s not found\n", argv[last]);
			exit(1);
		}

		parseHeader();
		printf("Loaded %s, %d subsongs\n", argv[last], sh.songs);
	}

	console_interface();
	return 0;
}
