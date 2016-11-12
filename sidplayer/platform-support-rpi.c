#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "bcm2835.h"
#include "platform-support.h"
#include "platform-support-common.h"

static void rpi_usleep(u_int32_t us) {
    usleep(us);
}

static void rpi_shutdown() {
	// function selection register 2, GPIO 20-29
	uint32_t fsel = *bcm2835_registers.gpio_fsel2;
	// clear 21 and 27
	fsel &= 0xff1fffc7;
	//  enable input on 21 and 27
	*bcm2835_registers.gpio_fsel2 = fsel;
}

static void sid_write(uint8_t reg, uint8_t data) {
	uint8_t data_01 = data & 0x3;
	uint8_t data_23 = (data & 0xC) >> 2;
	uint8_t data_4567 = (data & 0xF0) >> 4;
	
	uint32_t set_pins = 0;
	set_pins |= data_01;		// compatability with v1
	set_pins |= data_01 << 2;	// and v2
	set_pins |= data_23 << 14;
	set_pins |= data_4567 << 22;

	set_pins |= (reg & 0x1F) << 7;	// SID Address
	
	// SET Pins
	*bcm2835_registers.gpio_output_set0 = set_pins;
	
	// Clear Pins
	uint32_t clear_pins = 0;
	clear_pins |= (1 << 17);	// SID read/Write
	clear_pins |= (1 << 4);		// SID CS
	
	while ((*bcm2835_registers.gpio_level0) & (1 << 18));	// wait for low
	*bcm2835_registers.gpio_output_clear0 = clear_pins;
	while (!((*bcm2835_registers.gpio_level0) & (1 << 18)));	// wait for high
	while ((*bcm2835_registers.gpio_level0) & (1 << 18));	// wait for low
	
	clear_pins = 0x03C0CF8F;	// Clear 0-3, 7-11, 14-15, 22-25
	set_pins  = (1 << 17);		// SID Read/write
	set_pins |= (1 << 4);		// SID CS
	*bcm2835_registers.gpio_output_set0 = set_pins;
	*bcm2835_registers.gpio_output_clear0 = clear_pins;
}

static void init_gpio(uint32_t base_addr) {
    bcm2835_registers.gpio_base = get_addr(base_addr + GPIO_OFFSET);

    /*
    0x1 as output on pins 0, 1, 2, 3, 4, 7, 8, 9, 10, 11, 14, 15, 17, 21, 22, 23, 24, 25, 27
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
    // clear 21, 22-25, 27
    fsel &= 0x3f1C0007;
    //  enable input on 21, 22-25, 27
    *bcm2835_registers.gpio_fsel2 = fsel;
    // enable output on 21, 22-25, 27
    fsel |= 0x209248;
    *bcm2835_registers.gpio_fsel2 = fsel;

    bcm2835_registers.gpio_output_set0 = bcm2835_registers.gpio_base + 7; // 0x1C;
    bcm2835_registers.gpio_output_set1 = bcm2835_registers.gpio_base + 8; // 0x20;
    bcm2835_registers.gpio_output_clear0 = bcm2835_registers.gpio_base + 10; // 0x28;
    bcm2835_registers.gpio_output_clear1 = bcm2835_registers.gpio_base + 11; // 0x2C;
    bcm2835_registers.gpio_level0 = bcm2835_registers.gpio_base + 13; // 0x34;

    // enable power (pins 21 and 27 for compat. with rev 1 and 2)
    *bcm2835_registers.gpio_output_clear0 = 0x08200000;
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
static void init_pwm(uint32_t base_addr) {
    unsigned int pwm_pwd = (0x5A << 24);
    bcm2835_registers.pwm_base = get_addr(base_addr + PWM_OFFSET);
    bcm2835_registers.pwm_rng1 = bcm2835_registers.pwm_base + 4;
    bcm2835_registers.pwm_dat1 = bcm2835_registers.pwm_base + 5;

    bcm2835_registers.clock_base = get_addr(base_addr + CLOCK_OFFSET);
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

static void init_rpi() {
    set_realtime();

    if (open_mem() != EXIT_SUCCESS) {
        exit(EXIT_FAILURE);
    }

    init_gpio(BCM2835_BASE);
    init_pwm(BCM2835_BASE);
}

static void init_rpi2() {
    set_realtime();

    if (open_mem() != EXIT_SUCCESS) {
        exit(EXIT_FAILURE);
    }

    init_gpio(BCM2836_BASE);
    init_pwm(BCM2836_BASE);
}

struct reveller_platform rpi_platform = {
    .init = &init_rpi,

    .debug = &common_platform_debug,
    .abort = &common_platform_abort,

    .pause = &common_pause,
    .resume = &common_resume,
    .usleep = &rpi_usleep,
    .shutdown = &rpi_shutdown,

    .read = &common_platform_read_source,

    .sid_block_start = &common_sid_block_start,
    .sid_block_end = &common_sid_block_end,
    .sid_write = &sid_write,

    .platform_id = "Raspberry Pi",
};

struct reveller_platform rpi2_platform = {
    .init = &init_rpi2,

    .debug = &common_platform_debug,
    .abort = &common_platform_abort,

    .pause = &common_pause,
    .resume = &common_resume,
    .usleep = &rpi_usleep,
    .shutdown = &rpi_shutdown,

    .read = &common_platform_read_source,

    .sid_block_start = &common_sid_block_start,
    .sid_block_end = &common_sid_block_end,
    .sid_write = &sid_write,

    .platform_id = "Raspberry Pi 2/3",
};
