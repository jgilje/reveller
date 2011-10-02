// VIC-II emulering
// konsentrerer meg atter engang _KUN_ om Interrupt funksjonen
// og skal i teorien være enkel.

// - Finn ut om Interrupt er slått av eller på
// - Ved PAL er denne 50Hz, ved NTSC er denne 60Hz

#include "vic.h"


void vicInit(void) {
	memset(&vicReg, 0, sizeof(vicRegister));
}

void vicWrite(unsigned char addr, unsigned char data) {
	switch (addr) {
		case 0x19:			// Interrupt Register
			vicReg.idr &= (~data & 0xf);
//			if (vicReg.idr == 0x80) {
//				vicInterrupt = 0;
//			} else {
//				vicInterrupt = 1;
//			}
			break;
		case 0x1a:			// Interrupt Enable
			vicReg.icr = (data & 0xf);
			if (vicReg.icr != 0) {
				vicInterrupt = 1;
				c64_set_freq_vic(50);
				c64_start_freq_vic();
			} else {
				vicInterrupt = 0;
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

void vicNextInterrupt(void) {
	if (vicReg.icr & 0x1) {		// vi sjekker kun VBI
		vicReg.idr |= 0x81;
	}
}
