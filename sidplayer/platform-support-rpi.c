#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "bcm2835.h"

void platform_usleep(u_int32_t us) {
    usleep(us);
}

void c64_sid_block_start(void) {
}

void c64_sid_block_end(void) {
}

void c64_sid_write(uint8_t reg, uint8_t data) {
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
