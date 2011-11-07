#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#include "sidheader.h"

FILE* inputSidFile;
FILE* outfile;

void openOutput() {
    outfile = fopen("sidstream.zzz", "wb");
    return;
}

void printWelcome() {
	printf("SID Companiet - 6510 Emulator\n");
	PrintOpcodeStats();
}

char* nextToken(char* in) {
	int i;
	for (i = 0; i < strlen(in); i++) {
		if (in[i] == ' ') {
			in[i] = 0x0;
			return(&in[i + 1]);
		}
	}
	return NULL;
}

// SangSpilling foregår ved å sette registerene A (X, Y) før en kaller opp interpreteren
int main(int argc, char **argv) {
	char input[256];
	char command[8];
	char *args;
	int i = 0;
	int song = 0;
	int interactive = 0;

	printWelcome();
	openOutput();

	// sjekk opp argv[1]
	if (argv[1]) {
		inputSidFile = fopen(argv[1], "rb");
		if (inputSidFile == 0) {
			printf("ERROR: File %s not found\n", argv[1]);
			exit(1);
		}
		fseek(inputSidFile, 0, SEEK_END);
		printf("Inputfile is %ld bytes\n", ftell(inputSidFile));
		
		setSubSong(0);
	}
	
	while(1) {
		printf("6510> ");
		input[0] = 0x0;
		fgets(input, 256, stdin);
		
		if (input[0] == 0x0) { printf("\n"); exit(0); }
		input[strlen(input) - 1] = 0x0;
		
		args = nextToken(input);
		
		if (! strcmp(input, "help") || ! strcmp(input, "h")) {
			printf("\n6510 Commands\n"
				   "(h)elp\n"
				   "(p)lay [iterations]\n"
				   "(s)ong <subgsong>\n"
				   "(l)oad <file>\n"
				   "(d)umpmem\n"
				   "(q)uit\n"
				   );
		} else if (! strcmp(input, "load") || ! strcmp(input, "l")) {
			inputSidFile = fopen(args, "rb");
			if (inputSidFile == 0) {
				printf("ERROR: File %s not found\n", args);
			}
			
			setSubSong(0);
		} else if (! strcmp(input, "play") || ! strcmp(input, "p")) {
			int i;
			unsigned short addr;
			if (! inputSidFile) {
				printf("No SID is loaded\n");
				continue;
			}

			if (args) {
				i = strtoul(args, NULL, 0);
				if (i < 0) {
					printf("Invalid song\n");
					continue;
				}
			} else {
				// legg opp til uendelig spilling
				// Windows har visst _en_ fordel i dette tilfellet (Shiver me Timbers)
#ifdef WIN32
                while (! kbhit()) {
					c64_play();
                }
                
                getch();
#else
				struct termios currentTerm;
				struct termios originalTerm;
				int originalFcntl;
				
				tcgetattr(STDIN_FILENO, &currentTerm);
				originalTerm = currentTerm;
				
				currentTerm.c_lflag &= ~(ECHO | ICANON | IEXTEN);
				currentTerm.c_cc[VMIN] = 1;
				currentTerm.c_cc[VTIME] = 0;
				tcsetattr(STDIN_FILENO, TCSANOW, &currentTerm);
				originalFcntl = fcntl(STDIN_FILENO, F_GETFL, 1);
				fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
				
				printf("Playing... (any key to stop)...");
				
				int32_t skipped_time = 0;
				while (getc(stdin) < 0) {
					int debug_skip = 0;
					// uint32_t next = c64_play();
					int32_t next = c64_play();
					if (next < 0) {
						skipped_time -= next;
						continue;
					}
					
					if (skipped_time != 0) {
						c64_debug("next: %d, skipped_time mod: %d\n", next, skipped_time);
						debug_skip = 1;
					}
					next += skipped_time;
					next = next * ((float) sh.hz / 1000000.0f);
					if (debug_skip) {
						c64_debug("final next: %d\n", next);
					}
					
					skipped_time = 0;
					
					printf("usleep(%d)\n", next);
				}
				
				fcntl(STDIN_FILENO, F_SETFL, originalFcntl);
				tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTerm);
				printf("\n");
#endif

				continue;
			}

			// PLAY
			printf("Starting PlayAddr %d times\n", i);
			{
			    int j;
			    for (j = 0; j < i; j++) {
					int32_t next = c64_play();
					printf("usleep(%d)\n", next);
			    }
			}
		} else if (! strcmp(input, "song") || ! strcmp(input, "s")) {
			int i;
			if (! inputSidFile) {
				printf("No SID is loaded\n");
				continue;
			}

			if (args) {
				i = strtoul(args, NULL, 0);
				if (i < 0) {
					printf("Invalid song\n");
					continue;
				}

				song = i;
				setSubSong(song);
				printf("Song is now %d\n", song);
			}
		} else if (! strcmp(input, "dump") || ! strcmp(input, "d")) {
			dumpMem();
		} else if (! strcmp(input, "quit") || ! strcmp(input, "q")) {
			// ikke så nøye her, vi er ferdige anyway
			fflush(NULL);
			printf("Bye\n");
			exit(0);
		} else if (interactive && inputSidFile) {
			printf("Starting PlayAddr 1 time\n");
			interpret(1, sh.playAddress);
		}
	}
}
