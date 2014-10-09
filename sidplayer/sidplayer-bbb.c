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
#include "am335x.h"
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

/*
 * enable the required subsystems
 * before they are enabled, register accesses causes memory errors
 * (ex. "Bus Error", "Unhandled fault: external abort on non-linefetch (0x1018) at 0xXXXXXXXX")
 */
void initPeripherals(void) {
	am335x_registers.clock_manager = get_addr(CM_PER);
	
	uint32_t gpio_1_status = *(am335x_registers.clock_manager + CM_PER_GPIO1_CLKCTRL);
	uint32_t gpio_2_status = *(am335x_registers.clock_manager + CM_PER_GPIO2_CLKCTRL);
	uint32_t pwm_1_status = *(am335x_registers.clock_manager + CM_PER_EPWMSS1_CLKCTRL);
	
	if (gpio_1_status & CLKCTRL_STATUS_DISABLED) {
		*(am335x_registers.clock_manager + CM_PER_GPIO1_CLKCTRL) = CLKCTRL_MODE_ENABLE;
	}
	if (gpio_2_status & CLKCTRL_STATUS_DISABLED) {
		*(am335x_registers.clock_manager + CM_PER_GPIO2_CLKCTRL) = CLKCTRL_MODE_ENABLE;
	}
	if (pwm_1_status & CLKCTRL_STATUS_DISABLED) {
		*(am335x_registers.clock_manager + CM_PER_EPWMSS1_CLKCTRL) = CLKCTRL_MODE_ENABLE;
	}
}

void initGPIO(void) {
	uint32_t oe;
	am335x_registers.control_module = get_addr(CONTROL_MODULE);

	// TODO verify we need to update the CONTROL_MODULE
	/*
	printf("%x %x %x\n", CONTROL_MODULE, am335x_registers.control_module, am335x_registers.control_module + (0x870/4));
	uint32_t z = *(am335x_registers.control_module + (0x878 / 4));
	printf("current contents of 0x870: %x\n", z);
	*/
	am335x_registers.gpio_0 = get_addr(GPIO_0);
	am335x_registers.gpio_0_clear = am335x_registers.gpio_0 + (GPIO_CLEAR_OFFSET);
	am335x_registers.gpio_0_set = am335x_registers.gpio_0 + (GPIO_SET_OFFSET);
	am335x_registers.gpio_0_oe = am335x_registers.gpio_0 + (GPIO_OE_OFFSET);
	oe = *am335x_registers.gpio_0_oe;
	oe &= 0x3FFFF07F;	// output on pin 31, 30, 11 <-> 7
	*am335x_registers.gpio_0_oe = oe;

	am335x_registers.gpio_1 = get_addr(GPIO_1);
	am335x_registers.gpio_1_clear = am335x_registers.gpio_1 + (GPIO_CLEAR_OFFSET);
	am335x_registers.gpio_1_set = am335x_registers.gpio_1 + (GPIO_SET_OFFSET);
	am335x_registers.gpio_1_oe = am335x_registers.gpio_1 + (GPIO_OE_OFFSET);
	oe = *am335x_registers.gpio_1_oe;
	oe &= 0xEFFFFFFF;	// output on pin 28
	*am335x_registers.gpio_1_oe = oe;
	
	am335x_registers.gpio_2 = get_addr(GPIO_2);
	am335x_registers.gpio_2_clear = am335x_registers.gpio_2 + (GPIO_CLEAR_OFFSET);
	am335x_registers.gpio_2_set = am335x_registers.gpio_2 + (GPIO_SET_OFFSET);
	am335x_registers.gpio_2_oe = am335x_registers.gpio_2 + (GPIO_OE_OFFSET);
	oe = *am335x_registers.gpio_2_oe;
	oe &= 0xFFFFC03F;	// output on pin 13 <-> 6
	*am335x_registers.gpio_2_oe = oe;

	// Clear POWER
	*am335x_registers.gpio_1_clear = (1 << 16);
	
	// Set RESET
	*am335x_registers.gpio_1_set = (1 << 28);
}

#define PWM_TBCTL (0x0 / 2)
#define PWM_TBSTS (0x2 / 2)
#define PWM_TBCNT (0x8 / 2)
#define PWM_TBPRD (0xA / 2)
#define PWM_CMPCTL (0xE / 2)
#define PWM_CMPA  (0x12 / 2)
#define PWM_AQCTLA (0x16 / 2)
void initPWM(void) {
	volatile uint32_t* pwm = get_addr(PWM_1);
	volatile uint16_t* tbctl_reg = (uint16_t*)(pwm + EPWM_OFFSET) + PWM_TBCTL;
	volatile uint16_t* tbsts_reg = (uint16_t*)(pwm + EPWM_OFFSET) + PWM_TBSTS;
	volatile uint16_t* tbcnt_reg = (uint16_t*)(pwm + EPWM_OFFSET) + PWM_TBCNT;
	volatile uint16_t* tbprd_reg = (uint16_t*)(pwm + EPWM_OFFSET) + PWM_TBPRD;
	volatile uint16_t* cmpctl_reg = (uint16_t*)(pwm + EPWM_OFFSET) + PWM_CMPCTL;
	volatile uint16_t*   cmpa_reg = (uint16_t*)(pwm + EPWM_OFFSET) + PWM_CMPA;
	volatile uint16_t* aqctla_reg = (uint16_t*)(pwm + EPWM_OFFSET) + PWM_AQCTLA;
	
	uint32_t tbctl_tbsts = *(pwm + EPWM_OFFSET + PWM_TBCTL);
	uint16_t tbctl = *tbctl_reg;
	uint16_t tbsts = *tbsts_reg;
	uint16_t tbcnt = *tbcnt_reg;
	uint16_t tbprd = *tbprd_reg;
	uint16_t cmpctl = *cmpctl_reg;
	uint16_t   cmpa = *cmpa_reg;
	printf("%p %p\n", tbctl_reg, tbsts_reg);
	printf("Combined: %x, CTL: %x, STS: %x\n", tbctl_tbsts, tbctl, tbsts);
	
	printf("%x %x\n", tbcnt, tbprd);
	printf("%x %x\n", cmpctl, cmpa);
	
	/*
	TBCTL[CNTLDE]
	TBCTL[SWFSYNC]
	*/
	
	/*
	Counter-compare (CC)
		• Specify the PWM duty cycle for output EPWMxA and/or output EPWMxB
		• Specify the time at which switching events occur on the EPWMxA or EPWMxB output
	*/
	
	/* time based 1MHz PWM */
	/* Set HSPCLKDIV: /1, CTRMODE: Up-count */
	//tbctl &= 0xfc7c;
	//printf("mod. tbctl: %x\n", tbctl);
	//*tbctl_reg = tbctl;
	// *tbcnt_reg = 500;
	/* Set Period */
	//*tbprd_reg = 50;
	/* Action when the counter equals the period: Toggle EPWMxA output */
	//printf("aqctla_reg: %x\n", *aqctla_reg);
	//*aqctla_reg = 0x3;
	
	/* Set HSPCLKDIV: /1 */
	tbctl &= 0xfc7f;
	*tbctl_reg = tbctl;
	*tbprd_reg = 101;
	*cmpa_reg = 51;
	*aqctla_reg = (0x2 << 4) | 0x1;
	
	while (*tbcnt_reg < 101);
	while (*tbcnt_reg < 101);
	
	exit(0);
}

void printWelcome(void) {
	printf("SID Companiet - 6510 Emulator\n");
	printf("\tLinux Hosted for Beaglebone Black\n");
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
	initPeripherals();
	initGPIO();
	initPWM();

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
