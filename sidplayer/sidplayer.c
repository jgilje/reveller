#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <sys/types.h>

#include "6510.h"
#include "console-interface.h"

#include "platform-support-common.h"

void printWelcome() {
    printf("Reveller SID, running on %s\n", reveller->platform_id);
    c64_printOpcodeStats();
}

int main(int argc, char **argv) {
    (void) argc;

    detect_platform();
    reveller->init();

	printWelcome();
	
	if (argv[1]) {
        reveller_input_file = fopen(argv[1], "rb");
        if (reveller_input_file == 0) {
			printf("ERROR: File %s not found\n", argv[1]);
			exit(1);
		}

		parseHeader();
		printf("Loaded %s, %d subsongs\n", argv[1], sh.songs);
	}

	console_interface();
	return 0;
}
