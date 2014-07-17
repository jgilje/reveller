/*
 * This version of the sidplayer is a complete standalone version
 * reading input from pushbuttons on the databus, and outputs info
 * using a lcd-display also connected to the databus.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "6510.h"
#include "insane-menu.h"

#include "sc2410.h"
#include "lcd-hammer.h"
#include "platform-support.h"

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
	s3c2410_registers.v_gpio_c_pullup = s3c2410_registers.v_gpio_base + (GPCPULLUP & MAP_MASK);

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

#define SID_HZ_PAL_CONVERSION (1000000/985248)

#include <sched.h>
void set_realtime(void) {
	struct sched_param param;
	param.sched_priority = 20;
	if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
		printf("Failed to get RealTime priority");
	}
}

int menu_callback_read(void) {
	static int last = -1;
	
	int ret;
	uint32_t c_data;
	uint32_t c_conf = *(REG s3c2410_registers.v_gpio_c_conf);
	
	*(REG s3c2410_registers.v_gpio_c_data) = CS_ALL & ~CS_BTN;
	
	*(REG s3c2410_registers.v_gpio_c_conf) = c_conf & 0x00ff; // data ports as input
	c_data = *(REG s3c2410_registers.v_gpio_c_data);
	*(REG s3c2410_registers.v_gpio_c_conf) = c_conf;
	
	c_data &= 0xff00;
	c_data = c_data >> DATA_BUS_SHIFT;
	c_data = ~c_data;
	*(REG s3c2410_registers.v_gpio_c_data) = CS_ALL;
	
	// printf("read: %x\n", c_data);
	
	if (c_data & 0x1) {
		ret = RIGHT;
	} else if (c_data & 0x2) {
		ret = UP;
	} else if (c_data & 0x4) {
		ret = LEFT;
	} else if (c_data & 0x16) {
		ret = DOWN;
	} else {
		ret = -1;
	}
	
	// printf("read pre: ret %d, last %d\n", ret, last);
	if (last != ret) {
		last = ret;
	} else {
		ret = -1;
	}
	
	// printf("read post: ret %d, last %d\n", ret, last);
	return ret;
}

void menu_callback_set_file(char* file) {
	inputSidFile = fopen(file, "rb");
	if (inputSidFile == 0) {
		printf("ERROR: File %s not found\n", file);
		exit(1);
	}
	fseek(inputSidFile, 0, SEEK_END);
	printf("Inputfile is %ld bytes\n", ftell(inputSidFile));
	
	setSubSong(0);
	printf("Loaded song %d of %d subsongs\n", sh.startSong, sh.songs);
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
	
	set_realtime();
	printWelcome();
	initGPIO();
	initPWM();

	// sjekk opp argv[1]
	if (argv[1]) {
		menu_init(argv[1]);
	} else {
		printf("Required parameter - top level directory not specified");
		exit(1);
	}
	
	if (argc > 2) {
		int z = strtol(argv[2], NULL, 10);
		printf("LCD reinit %d\n", z);
		lcd_reinit(z, z);
	} else {
		lcd_init();
	}
	
	while(1) {
		menu_run();
		
		if (inputSidFile != NULL) {
			int32_t next = c64_play();
			next = next * ((float) sh.hz / 1000000.0f);
            platform_usleep(next);
		} else {
			usleep(1000);
		}
	}
}
