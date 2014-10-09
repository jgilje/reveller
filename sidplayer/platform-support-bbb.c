#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "am335x.h"

void platform_usleep(uint32_t us) {
    usleep(us);
}

void platform_exit() {
    *am335x_registers.gpio_1_set = (1 << 16);
}

void c64_sid_block_start(void) {
}

void c64_sid_block_end(void) {
}

void c64_sid_write(uint8_t reg, uint8_t data) {
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

void c64_set_freq_vic(uint32_t hz) {
//	printf("Program set VIC timer to %d Hz\n", hz);
}
void c64_start_freq_vic(void) {
//	printf("Program started VIC timer\n");
}

void c64_set_freq_cia_a_irq(uint32_t hz) {
// 	printf("Program set CIA-A (IRQ) timer to %d Hz\n", hz);
}
void c64_start_freq_cia_a_irq(void) {
//	printf("Program started CIA-A (IRQ) timer\n");
}

void c64_set_freq_cia_a_nmi(uint32_t hz) {
//	printf("Program set CIA-A (NMI) timer to %d Hz\n", hz);
}
void c64_start_freq_cia_a_nmi(void) {
//	printf("Program started CIA-A (NMI) timer\n");
}

