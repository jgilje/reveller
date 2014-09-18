#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "sc2410.h"

extern FILE *sid_kernel_timer;

void platform_usleep(uint32_t usec) {
    if (usec > 20971) {
        printf("sidplayer: usec was %d, limiting to 20950\n", usec);
        usec = 20950;
    }

    ioctl(fileno(sid_kernel_timer), usec);
}

void c64_sid_block_start(void) {
}

void c64_sid_block_end(void) {
}

void c64_sid_write(uint8_t reg, uint8_t data) {
	uint32_t gpc_data = (data << DATA_BUS_SHIFT);	// sid data
	gpc_data |= CS_ALL;
	
	uint32_t gpe_data = (reg << 11);	// ignore bits written to 14 and 15
	gpe_data &= 0x3800;
	uint32_t gpg_data = (reg >> 1);
	gpg_data &= 0xc;
	
	*(REG s3c2410_registers.v_gpio_e_data) = gpe_data;
	*(REG s3c2410_registers.v_gpio_g_data) = gpg_data;
	
	while (*(REG s3c2410_registers.v_gpio_c_data) & CS_CLK);			// wait for low
	gpc_data &= ~CS_SID;	// sid cs
	*(REG s3c2410_registers.v_gpio_c_data) = gpc_data;
	
	while (!(*(REG s3c2410_registers.v_gpio_c_data) & CS_CLK));		// wait for high
	while (*(REG s3c2410_registers.v_gpio_c_data) & CS_CLK);			// wait for low
	
	*(REG s3c2410_registers.v_gpio_c_data) = CS_ALL;
	*(REG s3c2410_registers.v_gpio_e_data) = 0;
	*(REG s3c2410_registers.v_gpio_g_data) = 0;
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

