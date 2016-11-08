#include "console-interface.h"
#include "platform-support.h"
#include "6510.h"

#if defined unix || (defined(__APPLE__) && defined(__MACH__))
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#endif

#if defined _WIN32
#include <conio.h>
#endif

void continuosPlay(void) {
    c64_sid_resume();

#if defined unix || (defined(__APPLE__) && defined(__MACH__))
	struct termios currentTerm;
	struct termios originalTerm;
	
    tcgetattr(STDIN_FILENO, &currentTerm);
    originalTerm = currentTerm;
    currentTerm.c_lflag &= ~(ECHO | ICANON | IEXTEN);
    currentTerm.c_cc[VMIN] = 1;
    currentTerm.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &currentTerm);

    printf("Playing... (any key to stop)...");
    fflush(stdout);

    if (reveller->write_handle) {
        struct pollfd pollfds[2];
        memset(&pollfds, 0, sizeof(struct pollfd) * 2);
        pollfds[0].fd = STDIN_FILENO;
        pollfds[0].events = POLLIN;

        pollfds[1].fd = reveller->write_handle();
        pollfds[1].events = POLLOUT;

        int ok = 1;
        while (ok) {
            int ret = poll(&pollfds[0], 2, -1);
            if (ret > 0) {
                if (pollfds[0].revents & POLLIN) {
                    ok = 0;
                }
                if (pollfds[1].revents & POLLOUT) {
                    int32_t next = c64_play();
                    reveller->usleep(next);
                }
            } else {
                reveller->abort("Failed poll() in continuosPlay()\n");
                ok = 0;
            }
        }
    } else {
        int originalFcntl = fcntl(STDIN_FILENO, F_GETFL, 1);
        if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK) < 0) {
            reveller->abort("Failed to set stdin nonblocking\n");
        }

        while (getc(stdin) < 0) {
            int32_t next = c64_play();
            reveller->usleep(next);
        }

        if (fcntl(STDIN_FILENO, F_SETFL, originalFcntl) < 0) {
            reveller->abort("Failed to restore stdin\n");
        }
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTerm);

    printf("\n");
	fflush(stdout);
#elif defined _WIN32
	while (! _kbhit()) {
		int32_t next = c64_play();
		next = next * ((float) sh.hz / 1000000.0f);
		platform_usleep(next);
	}
	_getch();

	printf("\n");
	fflush(stdout);
#else
	#error Unimplemented continuosPlay for this platform
#endif

    c64_sid_pause();
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
				   "(i)nteractive (toggle)\n"
				   "(q)uit\n"
				   );
		} else if (! strcmp(input, "load") || ! strcmp(input, "l")) {
            if (reveller_input_file) {
                fclose(reveller_input_file);
			}

            reveller_input_file = fopen(args, "rb");
            if (reveller_input_file == 0) {
				printf("ERROR: File %s not found\n", args);
			}
			
            c64_setSubSong(0);
		} else if (! strcmp(input, "play") || ! strcmp(input, "p")) {
			int i;
            if (! reveller_input_file) {
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

			// sanitize input (code scanners get happy)
			if (i > 0xffff) {
				i = 0xffff;
			}

			// START PLAY
			printf("Starting PlayAddr %d times\n", i);
			{
				int j;
				for (j = 0; j < i; j++) {
                    c64_sid_resume();
					c64_play();
                    c64_sid_pause();
                    reveller->usleep(1000000 / 55);
				}
			}
		} else if (! strcmp(input, "song") || ! strcmp(input, "s")) {
			int i;
            if (! reveller_input_file) {
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
                c64_setSubSong(song);
				printf("Song is now %d\n", song);
				continuosPlay();
			}
		} else if (! strcmp(input, "dump") || ! strcmp(input, "d")) {
            c64_dumpMem();
		} else if (! strcmp(input, "quit") || ! strcmp(input, "q")) {
            reveller->shutdown();

			fflush(NULL);
			printf("Bye\n");
			exit(0);
		} else if (! strcmp(input, "interactive") || ! strcmp(input, "i")) {
			if (interactive) {
				printf("Disabling interactive mode\n");
				interactive = 0;
                c64_sid_pause();
			} else {
				printf("Enabling interactive mode\n");
				interactive = 1;
                c64_sid_resume();
			}
        } else if (interactive && reveller_input_file) {
			printf("Starting PlayAddr 1 time\n");
			interpret(1, sh.playAddress);
		}

		fflush(stdout);
	}
}
