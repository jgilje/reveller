// VIC-II emulering
// konsentrerer meg atter engang _KUN_ om Interrupt funksjonen
// og skal i teorien være enkel.

// - Finn ut om Interrupt er slått av eller på
// - Ved PAL er denne 50Hz, ved NTSC er denne 60Hz

#include "vic.h"
#include "platform-support.h"

void vicWrite(unsigned char addr, unsigned char data) {
	switch (addr) {
		case 0x19:			// Interrupt Register
			vicReg.idr &= (~data & 0xf);
			break;
		case 0x1a:			// Interrupt Enable
			vicReg.icr = (data & 0xf);
			if (vicReg.icr != 0) {
				c64_set_freq_vic(50);
				c64_start_freq_vic();
				
				c64_vic_timer.latch = 19705;	// PAL latch, NTSC is different
				c64_vic_timer.counter = c64_vic_timer.latch;
				c64_vic_timer.enabled = 1;
			} else {
				c64_debug("Program Disabled VIC interrupts, and so should you!\n");
			}
			break;
		case 0x11:			// kontrollregister
		case 0x12:			// raster counter
			break;
		default:
			c64_debug("Unsupported VIC Write (%02x: %02x)\n", addr, data);
			exit(0);
	}
}

unsigned char vicRead(unsigned char addr) {
	switch (addr) {
//		case 0x19:
//			return vicReg.idr;
//			break;
//		case 0x1a:
//			return (vicReg.icr | 0xf0);
//			break;
		default:
			c64_debug("Unsupported VIC Read (%02x)\n", addr);
			exit(0);
	}
}

/*
void vicNextInterrupt(void) {
	if (vicReg.icr & 0x1) {		// vi sjekker kun VBI
		vicReg.idr |= 0x81;
	}
}
*/

void c64_vic_init(void) {
	memset(&vicReg, 0, sizeof(vicRegister));
	
	c64_vic_timer.latch = 0;
	c64_vic_timer.counter = 0;
	c64_vic_timer.enabled = 0;
	c64_vic_timer.interrupt = 0;
}

int32_t c64_vic_next_timer(void) {
	if (c64_vic_timer.enabled) {
		return c64_vic_timer.counter;
	}
	
	return -1;
}

void c64_vic_update_timer(int32_t next) {
	if (c64_vic_timer.enabled) {
		c64_vic_timer.counter -= next;
		if (c64_vic_timer.counter == 0) {
			c64_vic_timer.interrupt = 1;
			c64_vic_timer.counter = c64_vic_timer.latch;
		}
	}
}

uint32_t c64_vic_irq(void) {
	uint32_t ret = 0;
	if (c64_vic_timer.interrupt) {
		ret = 1;
		c64_vic_timer.interrupt = 0;
	}
	
	return ret;
}

