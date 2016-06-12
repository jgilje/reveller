#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sched.h>

#include "6510.h"
#include "sc2410.h"
#include "lcd-hammer.h"
#include "console-interface.h"

FILE *inputSidFile = NULL, *sid_kernel_timer = NULL;
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
        printf("Failed to unmap addr.: %p\n", addr);
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
    c64_printOpcodeStats();
}

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

void set_realtime(void) {
	struct sched_param param;
	param.sched_priority = 20;
	if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
		printf("Failed to get RealTime priority");
	}
}

// SangSpilling foregår ved å sette registerene A (X, Y) før en kaller opp interpreteren
int main(int argc, char **argv) {
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
		
        c64_setSubSong(0);
		printf("Loaded song %d of %d subsongs\n", sh.startSong, sh.songs);
	}
	
    console_interface();
    return 0;
}
