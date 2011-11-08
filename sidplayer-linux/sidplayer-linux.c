#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "6510.h"
#include "sidheader.h"

#include "sc2410.h"
#include "lcd.h"

FILE *inputSidFile, *sid_kernel_timer;
int fd_mem = -1;

s3c2410_registers_t s3c2410_registers;

#define USLEEP_CLK 0
#define USLEEP_NANOSLEEP 1
#define USLEEP_CLOCK_NANOSLEEP 2
#define USLEEP_SID_KERNEL_DRIVER 3
int usleep_function = USLEEP_SID_KERNEL_DRIVER;

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
	s3c2410_registers.v_gpio_base = get_addr(GPACON);
	s3c2410_registers.v_gpio_b_conf = s3c2410_registers.v_gpio_base + (GPBCON & MAP_MASK);
	s3c2410_registers.v_gpio_c_conf = s3c2410_registers.v_gpio_base + (GPCCON & MAP_MASK);
	s3c2410_registers.v_gpio_c_data = s3c2410_registers.v_gpio_base + (GPCDAT & MAP_MASK);
	s3c2410_registers.v_gpio_e_conf = s3c2410_registers.v_gpio_base + (GPECON & MAP_MASK);
	s3c2410_registers.v_gpio_e_data = s3c2410_registers.v_gpio_base + (GPEDAT & MAP_MASK);
	s3c2410_registers.v_gpio_g_conf = s3c2410_registers.v_gpio_base + (GPGCON & MAP_MASK);
	s3c2410_registers.v_gpio_g_data = s3c2410_registers.v_gpio_base + (GPGDAT & MAP_MASK);
	
	// gpio_c: pin 0,1 as input (read clk), rest as outputs
	// gpio_c_(8-15) is the data bus
	*(REG s3c2410_registers.v_gpio_c_conf) = 0x55555550;
	*(REG s3c2410_registers.v_gpio_c_data) = CS_ALL;		// all CS pins to high
	
	// gpio e 11-15 as outputs (problems reading from 14-15, so they are unused)
	// using gpio_e_(11,12,13) for SID addressing
	*(REG s3c2410_registers.v_gpio_e_conf) = 0x55400000;
	
	// all gpio g as outputs
	// using gpio_g_(2,3) for SID addressing
	*(REG s3c2410_registers.v_gpio_g_conf) = 0x55555555;
}

// 1.023MHz (PAL)
// 0.985MHz (NTSC)
void initPWM(void) {
	s3c2410_registers.t_base = get_addr(TCFG0);
	s3c2410_registers.v_tcfg1  = s3c2410_registers.t_base +  (TCFG1 & MAP_MASK);
	s3c2410_registers.v_tcon   = s3c2410_registers.t_base +   (TCON & MAP_MASK);
	s3c2410_registers.v_tcntb0 = s3c2410_registers.t_base + (TCNTB0 & MAP_MASK);
	s3c2410_registers.v_tcmpb0 = s3c2410_registers.t_base + (TCMPB0 & MAP_MASK);
	s3c2410_registers.v_tcntb1 = s3c2410_registers.t_base + (TCNTB1 & MAP_MASK);
	
	// tclk0 out
	uint32_t res = *(REG s3c2410_registers.v_gpio_b_conf);
	uint32_t w = res;
	w |= 0x2;
	*(REG s3c2410_registers.v_gpio_b_conf) = w;
	
	// timer1 prescaler=>1/16, timer1 will now run at 3.125MHz
	w = *(REG s3c2410_registers.v_tcfg1);
	w &= ~0xf0;
	w |= 0x30;
	*(REG s3c2410_registers.v_tcfg1) = w;
	
	// counter and compare to 24 (0x18) & 12 (0xC)
	// from listening 26-13 SOUNDS correct
	*(REG s3c2410_registers.v_tcntb0) = 25;
	*(REG s3c2410_registers.v_tcmpb0) = 13;
	
	// enable timer0
	res = *(REG s3c2410_registers.v_tcon);
	w = res & 0xffffff00;
	*(REG s3c2410_registers.v_tcon) = w | 0xa;
	usleep(1000);
	*(REG s3c2410_registers.v_tcon) = w | 0x9;
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

/*
void usleep_sid_clk(uint32_t cycles) {
	void* v_gpc_data = v_gpio_base + (GPCDAT & MAP_MASK);
	int i;
	
	for (i = 0; i < cycles; i++) {
		while (*(REG v_gpc_data) & CS_CLK);			// wait for low
		while (!(*(REG v_gpc_data) & CS_CLK));		// wait for high
	}
}

void usleep_nanosleep(uint32_t cycles) {
	struct timespec time;
	
	time.tv_sec = 0;
	time.tv_nsec = cycles*1000;
	
	if (nanosleep(&time, NULL) != 0) {
		printf("clock_nanosleep() failed\n");
	}
}

void usleep_clock_nanosleep(uint32_t cycles) {
	struct timespec time;
	
	time.tv_sec = 0;
	time.tv_nsec = cycles*1000;
	
	if (clock_nanosleep(CLOCK_REALTIME, 0, &time, NULL) != 0) {
		printf("clock_nanosleep() failed\n");
	}
}

#define NS_IN_S 1000000000
void usleep_burn(uint32_t cycles) {
	struct timespec now, start;
	int d = 0;
	int cycles_ns = cycles*1000;
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
	
	while (d < cycles_ns) {
		d = 0;
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
		if (now.tv_sec != start.tv_sec) {
			d += NS_IN_S;
		}
		d += (now.tv_nsec - start.tv_nsec);
	}
}
*/

void print_song(void) {
	char songname[21];
	char songauthor[21];
	char songsubsong[21];
	char copyright[21];
	snprintf(songname, 21, "%s", sh.name);
	snprintf(songauthor, 21, "%s", sh.author);
	snprintf(copyright, 21, "(C) %s", sh.released);
	snprintf(songsubsong, 21, "Song %d of %d", c64_current_song+1, sh.songs);
	
	lcd_clear();
	lcd_gotoxy(0, 0);
	lcd_puts(songname);
	lcd_gotoxy(0, 1);
	lcd_puts(songauthor);
	lcd_gotoxy(0, 2);
	lcd_puts(copyright);
	lcd_gotoxy(0, 3);
	lcd_puts(songsubsong);
}

void usleep_sid_kernel_timer(uint32_t usec) {
	char buf[1];
	// enable timer1
	uint32_t res = *(REG s3c2410_registers.v_tcon);
	uint32_t w = res & 0xfffff0ff;
	
	uint32_t count = (usec * 3125) / 1000;
	*(REG s3c2410_registers.v_tcntb1) = count;
	*(REG s3c2410_registers.v_tcon) = w | 0x200;	// timer1: one-shot, inverter-off, update tcntb1, stopped
	*(REG s3c2410_registers.v_tcon) = w | 0x100;	// clear update-tcntb1, start
	
	fread(buf, 1, 1, sid_kernel_timer);
}

#define SID_HZ_PAL_CONVERSION (1000000/985248)
void continuosPlay(void) {
	print_song();
	
	struct timespec b, a;
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
		clock_gettime(CLOCK_REALTIME, &b);
		int32_t next = c64_play();
		if (next < 0) {
			skipped_time -= next;
			continue;
		}
		next += skipped_time;
		next = next * ((float) sh.hz / 1000000.0f);
		skipped_time = 0;
		clock_gettime(CLOCK_REALTIME, &a);
		
		long emulator_time = 0;
		if (a.tv_sec != b.tv_sec) {
			emulator_time += 1000000000;
		}
		emulator_time += (b.tv_nsec - a.tv_nsec);
		emulator_time /= 1000;
		
		int n = next + emulator_time;
		// printf("%d = %d(%d) + %d + %d\n", n, sh.hz/next, next, usleep_bias, emulator_time);
		
		if (n < 0) n = 1;
		
		/*
		if (usleep_function == USLEEP_CLK) {
			usleep_sid_clk(n);
		} else if (usleep_function == USLEEP_NANOSLEEP) {
			usleep_nanosleep(n);
		} else if (usleep_function == USLEEP_CLOCK_NANOSLEEP) {
			usleep_clock_nanosleep(n);
		} else if (usleep_function == USLEEP_SID_KERNEL_DRIVER) {
			usleep_sid_kernel_timer(n);
		}
		*/
		usleep_sid_kernel_timer(n);
	}
	
	fcntl(STDIN_FILENO, F_SETFL, originalFcntl);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTerm);
	printf("\n");
}

#include <sched.h>
void set_realtime(void) {
	struct sched_param param;
	param.sched_priority = 20;
	if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
		printf("Failed to get RealTime priority");
	}
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
	
	sid_kernel_timer = fopen("/dev/sid_timer", "r");
	if (sid_kernel_timer == NULL) {
		printf("Could not get /dev/sid_timer\n");
		return -1;
	}
	
	printWelcome();
	initGPIO();
	initPWM();
	lcd_init();

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
		printf("Loaded song %d of %d subsongs\n", sh.startSong, sh.songs);
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
		} else if (! strcmp(input, "pwm")) {
			int i = strtoul(args, NULL, 0);
			args = nextToken(args);
			int j = strtoul(args, NULL, 0);
			printf("Set PWM counter 0x%x - compare 0x%x\n", i, j);
			setPWM(i, j);
		} else if (! strcmp(input, "lcd")) {
			int i = strtoul(args, NULL, 0);
			args = nextToken(args);
			int j = strtoul(args, NULL, 0);
			printf("LCD reinit: e_delay %d, r_delay %d\n", i, j);
			lcd_reinit(i, j);
		} else if (! strcmp(input, "usleep_function")) {
			usleep_function = strtol(args, NULL, 0);
			printf("usleep_function is now %d\n", usleep_function);
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
