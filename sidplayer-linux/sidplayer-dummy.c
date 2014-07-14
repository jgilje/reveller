#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "6510.h"
#include "console-interface.h"

FILE *inputSidFile = NULL;

void printWelcome() {
	printf("SID Companiet - 6510 Emulator\n");
	printf("\tDummy version\n");
	PrintOpcodeStats();
}

int main(int argc, char **argv) {
	printWelcome();
	
	if (argv[1]) {
		inputSidFile = fopen(argv[1], "rb");
		if (inputSidFile == 0) {
			printf("ERROR: File %s not found\n", argv[1]);
			exit(1);
		}
		fseek(inputSidFile, 0, SEEK_END);
		printf("Inputfile is %ld bytes\n", ftell(inputSidFile));
		
		setSubSong(0);
		printf("Loaded song %d of %d subsongs\n", sh.startSong, sh.songs);
	}

	console_interface();
	return 0;
}
