#include "console-interface.h"
#include "6510.h"

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

void platform_usleep(int32_t us);
FILE* inputSidFile;

void continuosPlay(void) {
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
	fflush(stdout);
	
	while (getc(stdin) < 0) {
		int32_t next = c64_play();
		next = next * ((float) sh.hz / 1000000.0f);
		platform_usleep(next);
	}
	
	fcntl(STDIN_FILENO, F_SETFL, originalFcntl);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTerm);
	printf("\n");
	fflush(stdout);
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

void console_interface(void) {
	char input[256];
	char *args;
	int song = 0;
	int interactive = 0;
	
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
				continuosPlay();
				continue;
			}

			// PLAY
			printf("Starting PlayAddr %d times (warning, no sleep)\n", i);
			{
			    int j;
			    for (j = 0; j < i; j++) {
					c64_play();
					usleep(1000000 / 55);
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
				continuosPlay();
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

		fflush(stdout);
	}
}
