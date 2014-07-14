#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sched.h>

#include "6510.h"
#include "bcm2835.h"
#include "console-interface.h"

FILE *inputSidFile = NULL, *sid_kernel_timer = NULL;
int fd_mem = -1;

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
	bcm2835_registers.gpio_base = get_addr(GPIO_BASE);

	/*
	0x1 as output on pins 0, 1, 2, 3, 4, 7, 8, 9, 10, 11, 14, 15, 17, 22, 23, 24, 25
	0x2 as function 5 on pin 18
	*/
	bcm2835_registers.gpio_fsel0 = bcm2835_registers.gpio_base;
	bcm2835_registers.gpio_fsel1 = bcm2835_registers.gpio_base + 1;
	bcm2835_registers.gpio_fsel2 = bcm2835_registers.gpio_base + 2;
	
	uint32_t fsel;
	
	// function selection register 0, GPIO 0-9
	fsel = *bcm2835_registers.gpio_fsel0;
	// clear all except 5 and 6
	fsel &= 0x1f8000;
	//  enable input on 0, 1, 2, 3, 4, 7, 8, 9
	*bcm2835_registers.gpio_fsel0 = fsel;
	// enable output on 0, 1, 2, 3, 4, 7, 8, 9
	fsel |= 0x9201249;
	*bcm2835_registers.gpio_fsel0 = fsel;
	
	// function selection register 1, GPIO 10-19
	fsel = *bcm2835_registers.gpio_fsel1;
	// clear all except 12, 13, 16 and 19
	fsel &= 0x381c0fc0;
	//  enable input on 10, 11, 14, 15, 17
	*bcm2835_registers.gpio_fsel1 = fsel;
	// enable output on 10, 11, 14, 15, 17
	// alt.fun. 5 on 18
	fsel |= 0x2209009;
	*bcm2835_registers.gpio_fsel1 = fsel;
	
	// function selection register 2, GPIO 20-29
	fsel = *bcm2835_registers.gpio_fsel2;
	// clear 22-25
	fsel &= 0x3ffc003f;
	//  enable input on 22-25
	*bcm2835_registers.gpio_fsel2 = fsel;
	// enable output on 22-25
	fsel |= 0x9240;
	*bcm2835_registers.gpio_fsel2 = fsel;
	
	bcm2835_registers.gpio_output_set0 = bcm2835_registers.gpio_base + 7; // 0x1C;
	bcm2835_registers.gpio_output_set1 = bcm2835_registers.gpio_base + 8; // 0x20;
	bcm2835_registers.gpio_output_clear0 = bcm2835_registers.gpio_base + 10; // 0x28;
	bcm2835_registers.gpio_output_clear1 = bcm2835_registers.gpio_base + 11; // 0x2C;
	bcm2835_registers.gpio_level0 = bcm2835_registers.gpio_base + 13; // 0x34;
	
	/*
	volatile uint32_t *g1, *g2;
	g1 = bcm2835_registers.gpio_base + 19;
	g2 = bcm2835_registers.gpio_base + 20;
	printf("GPREN0: %x, GPREN1: %x\n", *g1, *g2);
	printf("GPFEN0: %x, GPFEN1: %x\n", *(bcm2835_registers.gpio_base + 22), *(bcm2835_registers.gpio_base + 23));
	printf("GPHEN0: %x, GPHEN1: %x\n", *(bcm2835_registers.gpio_base + 25), *(bcm2835_registers.gpio_base + 26));
	printf("GPLEN0: %x, GPLEN1: %x\n", *(bcm2835_registers.gpio_base + 28), *(bcm2835_registers.gpio_base + 29));
	*/
}

// 1.023MHz (NTSC)
// 0.985MHz (PAL)
void initPWM(void) {
	unsigned int pwm_pwd = (0x5A << 24);
	bcm2835_registers.pwm_base = get_addr(PWM_BASE);
	bcm2835_registers.pwm_rng1 = bcm2835_registers.pwm_base + 4;
	bcm2835_registers.pwm_dat1 = bcm2835_registers.pwm_base + 5;
	
	bcm2835_registers.clock_base = get_addr(CLOCK_BASE);
	bcm2835_registers.clock_pwm_cntl = bcm2835_registers.clock_base + 40;
	bcm2835_registers.clock_pwm_div = bcm2835_registers.clock_base + 41;
	
	// Stop PWM clock
	*bcm2835_registers.clock_pwm_cntl = pwm_pwd | 0x01;
	usleep(110);
	
	// Wait for the clock to be not busy
	while ((*(bcm2835_registers.clock_pwm_cntl) & 0x80) != 0) {
		usleep(1);
	}

	// set the clock divider and enable PWM clock
	// hardcoded to PAL freqs. We get a clock on PWM of PLLD (500MHz) / 254 / 2= 0.984MHz,
	*bcm2835_registers.clock_pwm_div = pwm_pwd | (254 << 12);
	*bcm2835_registers.clock_pwm_cntl = pwm_pwd | 0x16;
	
	*bcm2835_registers.pwm_base = 0x80 | 1;

	*bcm2835_registers.pwm_rng1 = 2;
	*bcm2835_registers.pwm_dat1 = 1;
}

void printWelcome() {
	printf("SID Companiet - 6510 Emulator\n");
	printf("\tLinux Hosted for Raspberry Pi\n");
	PrintOpcodeStats();
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
	
	set_realtime();
	printWelcome();
	initGPIO();
	initPWM();
	printf("Peripheral setup complete\n");

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

	console_interface();
	return 0;
}
