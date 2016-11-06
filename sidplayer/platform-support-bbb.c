#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "am335x.h"
#include "platform-support.h"
#include "platform-support-common.h"

static void bbb_usleep(uint32_t us) {
    usleep(us);
}

static void shutdown() {
    *am335x_registers.gpio_1_set = (1 << 16);
    release_addr((void*) am335x_registers.clock_manager);
    release_addr((void*) am335x_registers.control_module);
    release_addr((void*) am335x_registers.gpio_0);
    release_addr((void*) am335x_registers.gpio_1);
    release_addr((void*) am335x_registers.gpio_2);
    release_addr((void*) am335x_registers.pwm);
}

static void sid_write(uint8_t reg, uint8_t data) {
	uint32_t set_pins_0 = 0;
	set_pins_0 |= ((reg & 0x1f) << 7);	// register pins from 7 <-> 11

	uint32_t set_pins_2 = 0;
	set_pins_2 |= (data << 6);		// pins 70 <-> 77

	// SET Pins
	*am335x_registers.gpio_0_set = set_pins_0;
	*am335x_registers.gpio_2_set = set_pins_2;
	
	// Clear Pins
	uint32_t clear_pins = 0;
	clear_pins |= (1 << 30);	// SID read/Write
	clear_pins |= (1 << 31);	// SID CS
	
	*am335x_registers.gpio_0_clear = clear_pins;

	/*
	while (*am335x_registers.pwm_tbcnt > 50);
	while (*am335x_registers.pwm_tbcnt < 25);
	*/
	int i = 0;
	while (i++ < 32);

	// Disable SID access first
	uint32_t set_pins = 0;
	set_pins |= (1 << 30);
	set_pins |= (1 << 31);
	*am335x_registers.gpio_0_set = set_pins;
	
	*am335x_registers.gpio_0_clear = 0xf80;		// clear pins 7 <-> 11
	*am335x_registers.gpio_2_clear = 0x3fc0;	// clear pins 70 <-> 77
}

static void init_peripherals() {
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

void init_gpio(void) {
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
    oe &= 0xEFFF7FFF;	// output on pin 16 and 28
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

void init_pwm(void) {
    am335x_registers.pwm = get_addr(PWM_1);
    volatile uint16_t* tbctl_reg = (uint16_t*)(am335x_registers.pwm + EPWM_OFFSET) + PWM_TBCTL;
    // volatile uint16_t* tbsts_reg = (uint16_t*)(am335x_registers.pwm + EPWM_OFFSET) + PWM_TBSTS;
    am335x_registers.pwm_tbcnt = (uint16_t*)(am335x_registers.pwm + EPWM_OFFSET) + PWM_TBCNT;
    volatile uint16_t* tbprd_reg = (uint16_t*)(am335x_registers.pwm + EPWM_OFFSET) + PWM_TBPRD;
    // volatile uint16_t* cmpctl_reg = (uint16_t*)(am335x_registers.pwm + EPWM_OFFSET) + PWM_CMPCTL;
    // volatile uint16_t*   cmpa_reg = (uint16_t*)(am335x_registers.pwm + EPWM_OFFSET) + PWM_CMPA;
    volatile uint16_t* aqctla_reg = (uint16_t*)(am335x_registers.pwm + EPWM_OFFSET) + PWM_AQCTLA;

    // uint32_t tbctl_tbsts = *(am335x_registers.pwm + EPWM_OFFSET + PWM_TBCTL);
    uint16_t tbctl = *tbctl_reg;
    // uint16_t tbsts = *tbsts_reg;
    // uint16_t tbcnt = *am335x_registers.pwm_tbcnt;
    // uint16_t tbprd = *tbprd_reg;
    // uint16_t cmpctl = *cmpctl_reg;
    // uint16_t   cmpa = *cmpa_reg;

    /*
    printf("%p %p\n", tbctl_reg, tbsts_reg);
    printf("Combined: %x, CTL: %x, STS: %x\n", tbctl_tbsts, tbctl, tbsts);

    printf("%x %x\n", tbcnt, tbprd);
    printf("%x %x\n", cmpctl, cmpa);
    */

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
    tbctl &= 0xfc7c;
    //printf("mod. tbctl: %x\n", tbctl);
    *tbctl_reg = tbctl;
    *am335x_registers.pwm_tbcnt = 500;
    /* Set Period */
    *tbprd_reg = 50;
    /* Action when the counter equals the period: Toggle EPWMxA output */
    // printf("aqctla_reg: %x\n", *aqctla_reg);
    *aqctla_reg = 0x3;

    /* Set HSPCLKDIV: /1 */
    //tbctl &= 0xfc7f;
    //*tbctl_reg = tbctl;
    //*tbprd_reg = 101;
    //*cmpa_reg = 51;
    //*aqctla_reg = (0x2 << 4) | 0x1;

    // while (*tbcnt_reg < 25);
    // while (*tbcnt_reg < 25);
}

static void init() {
    set_realtime();

    if (open_mem() != EXIT_SUCCESS) {
        exit(EXIT_FAILURE);
    }

    init_peripherals();
    init_gpio();
    init_pwm();
}

struct reveller_platform bbb_platform = {
    .init = &init,

    .debug = &common_platform_debug,
    .abort = &common_platform_abort,

    .usleep = &bbb_usleep,
    .shutdown = &shutdown,

    .read = &common_platform_read_source,
    .flush = &common_platform_flush,

    .sid_block_start = &common_sid_block_start,
    .sid_block_end = &common_sid_block_end,
    .sid_write = &sid_write,

    .platform_id = "Beaglebone Black",
};
