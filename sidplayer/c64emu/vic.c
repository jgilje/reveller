// VIC-II emulering
// konsentrerer meg atter engang _KUN_ om Interrupt funksjonen
// og skal i teorien være enkel.

// - Finn ut om Interrupt er slått av eller på
// - Ved PAL er denne 50Hz, ved NTSC er denne 60Hz

#include "vic.h"
#include "platform-support.h"

static vicRegister vicReg;

void c64_vic_write(unsigned char addr, unsigned char data) {
	switch (addr) {
		case 0x19:			// Interrupt Register
			vicReg.idr &= (~data & 0xf);
			break;
		case 0x1a:			// Interrupt Enable
			vicReg.icr = (data & 0xf);
			if (vicReg.icr != 0) {
				c64_vic_timer.latch = 19705;	// PAL latch, NTSC is different
				c64_vic_timer.counter = c64_vic_timer.latch;
				c64_vic_timer.enabled = 1;
			} else {
                reveller->debug("WARNING: Program Disabled VIC interrupts, and so should you!\n");
			}
			break;
		case 0x11:			// control register
		case 0x12:			// raster counter
		case 0x15:			// sprite enable
		case 0x20:			// border color
			break;
		default:
            reveller->abort("Unsupported VIC Write (%02x: %02x)\n", addr, data);
	}
}

unsigned char c64_vic_read(unsigned char addr) {
	switch (addr) {
//		case 0x19:
//			return vicReg.idr;
//			break;
//		case 0x1a:
//			return (vicReg.icr | 0xf0);
//			break;
		default:
            reveller->abort("Unsupported VIC Read (%02x)\n", addr);
	}
	
    reveller->abort("vicRead: Unreachable\n");
	return 0;
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
		if (next >= c64_vic_timer.counter) {
			c64_vic_timer.interrupt = 1;
			c64_vic_timer.counter = c64_vic_timer.latch - (next - c64_vic_timer.counter);
		} else {
			c64_vic_timer.counter -= next;
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

