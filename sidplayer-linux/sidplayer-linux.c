#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "sidheader.h"
#include "sc2410.h"

FILE* inputSidFile;
int fd_mem = -1;
void* v_gpio_base = NULL;

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
		printf("Failed to unmap addr.: %x\n", addr);
		exit(-1);
	}
}

void initGPIO(void) {
	v_gpio_base = get_addr(GPACON);
	void* v_gpio_c_conf = v_gpio_base + (GPCCON & MAP_MASK);
	void* v_gpio_c_data = v_gpio_base + (GPCDAT & MAP_MASK);
	void* v_gpio_e_conf = v_gpio_base + (GPECON & MAP_MASK);
	void* v_gpio_g_conf = v_gpio_base + (GPGCON & MAP_MASK);
	
	// gpio_c: pin 0,1 as input (read clk), rest as outputs
	// gpio_c_(8-15) is the data bus
	*(REG v_gpio_c_conf) = 0x55555550;
	*(REG v_gpio_c_data) = CS_ALL;		// all CS pins to high
	
	// gpio e 11-15 as outputs (problems reading from 14-15, so they are unused)
	// using gpio_e_(11,12,13) for SID addressing
	*(REG v_gpio_e_conf) = 0x55400000;
	
	// all gpio g as outputs
	// using gpio_g_(2,3) for SID addressing
	*(REG v_gpio_g_conf) = 0x55555555;
}

// 1.023MHz (PAL)
// 0.985MHz (NTSC)
void initPWM(void) {
	void* gpio_b_conf = get_addr(GPBCON);
	void* v_gpio_b_conf = gpio_b_conf + (GPBCON & MAP_MASK);
	
	void* tcfg0 = get_addr(TCFG0);
	void* v_tcon   = tcfg0 +   (TCON & MAP_MASK);
	void* v_tcntb0 = tcfg0 + (TCNTB0 & MAP_MASK);
	void* v_tcmpb0 = tcfg0 + (TCMPB0 & MAP_MASK);
	
	// tclk0 out
	uint32_t res = *(REG v_gpio_b_conf);
	uint32_t w = res;
	w |= 0x2;
	*(REG v_gpio_b_conf) = w;
	
	// counter and compare to 24 (0x18) & 12 (0xC)
	// from listening 26-13 SOUNDS correct
	*(REG v_tcntb0) = 26;
	*(REG v_tcmpb0) = 13;
	
	// enable timer0
	res = *(REG v_tcon);
	w = res & 0xffffff00;
	*(REG v_tcon) = w | 0xa;
	usleep(1000);
	*(REG v_tcon) = w | 0x9;
	
	release_addr(gpio_b_conf);
	release_addr(tcfg0);
}

void setPWM(uint8_t counter, uint8_t compare) {
	void* tcfg0 = get_addr(TCFG0);
	
	void* v_tcntb0 = tcfg0 + (TCNTB0 & MAP_MASK);
	void* v_tcmpb0 = tcfg0 + (TCMPB0 & MAP_MASK);

	*(REG v_tcntb0) = counter;
	*(REG v_tcmpb0) = compare;
	
	release_addr(tcfg0);
}

void printWelcome() {
	printf("SID Companiet - 6510 Emulator\n");
	printf("\tLinux Hosted. Dev 16\n");
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

	if((fd_mem = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
		printf("Could not get /dev/mem\n");
		return -1;
	}
	
	printWelcome();
	initPWM();
	initGPIO();

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
				
				while (getc(stdin) < 0) {
					IRQTrigger();
					usleep(1000000 / 55);
				}
				
				fcntl(STDIN_FILENO, F_SETFL, originalFcntl);
				tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTerm);
				printf("\n");

				continue;
			}

			// PLAY
			printf("Starting PlayAddr %d times\n", i);
			{
			    int j;
			    for (j = 0; j < i; j++) {
					IRQTrigger();
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
			}
		} else if (! strcmp(input, "dump") || ! strcmp(input, "d")) {
			dumpMem();
		} else if (! strcmp(input, "pwm")) {
			int i = strtoul(args, NULL, 0);
			args = nextToken(args);
			int j = strtoul(args, NULL, 0);
			printf("Set PWM counter 0x%x - compare 0x%x\n", i, j);
			setPWM(i, j);
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
