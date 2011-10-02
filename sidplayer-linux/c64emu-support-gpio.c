#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "sc2410.h"

extern FILE* inputSidFile;
extern void* v_gpio_base;

void c64_sid_block_start(void) {
	printf("=>\n");
}

void c64_sid_block_end(void) {
	printf("<=\n");
}

void c64_sid_write(uint8_t reg, uint8_t data) {
	void* v_gpc_data = v_gpio_base + (GPCDAT & MAP_MASK);
	void* v_gpe_data = v_gpio_base + (GPEDAT & MAP_MASK);
	void* v_gpg_data = v_gpio_base + (GPGDAT & MAP_MASK);
	
	// wait for bus, if already high - wait for low
	printf("=1\n");
	/*
	uint32_t clk = *((uint32_t *) v_gpc_data) & CS_CLK;
	if (clk) {
	    while(*((uint32_t *) v_gpc_data) & CS_CLK);
	}
	*/
    while(*((uint32_t *) v_gpc_data) & CS_CLK);
	printf("=2\n");
	
	uint32_t gpc_data = (data << 8);	// sid data
	gpc_data |= CS_ALL;
	gpc_data &= ~CS_SID;				// sid cs
	uint32_t gpe_data = (reg << 11);	// ignore bits written to 14 and 15
	gpe_data &= 0x3800;
	uint32_t gpg_data = (reg >> 1);
	gpg_data &= 0xc;
	
	*((uint32_t *) v_gpc_data) = gpc_data;
	*((uint32_t *) v_gpe_data) = gpe_data;
	*((uint32_t *) v_gpg_data) = gpg_data;
	
	// wait for bus
	printf("=3\n");
    while(!(*((uint32_t *) v_gpc_data) & CS_CLK));		// wait for high
    while(*((uint32_t *) v_gpc_data) & CS_CLK);			// wait for low
	printf("=4\n");
	
	*((uint32_t *) v_gpc_data) = CS_ALL;
}

void c64_set_freq_vic(uint32_t hz) {
	printf("Program set VIC timer to %d Hz\n", hz);
}
void c64_start_freq_vic(void) {
	printf("Program started VIC timer\n");
}

void c64_set_freq_cia_a_irq(uint32_t hz) {
	printf("Program set CIA-A timer to %d Hz\n", hz);
}
void c64_start_freq_cia_a_irq(void) {
	printf("Program started CIA-A timer\n");
}

void c64_set_freq_cia_a_nmi(uint32_t hz) {
    fprintf(stderr, "Exit at %s: %d\n", __FILE__, __LINE__);
    exit(1);
}
void c64_start_freq_cia_a_nmi(void) {
    fprintf(stderr, "Exit at %s: %d\n", __FILE__, __LINE__);
    exit(1);
}
