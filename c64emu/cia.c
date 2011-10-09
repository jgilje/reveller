// Endag!  ENDAG!!!
// står CIA på døra

// i første omgang konsentrerer vi på IRQ funksjonen


#include "cia.h"


void ciaInit() {
	int i;
	memset(&ciaRegister, 0, sizeof(ciaRegister));
	memset(&ciaTimers, 0, sizeof(ciaTimers));
	
	for (i = 0; i < 2; i++) {
		ciaTimers[i].counters[0] = 0xffff;
		ciaTimers[i].counters[1] = 0xffff;
		ciaTimers[i].latches[0] = 0xffff;
		ciaTimers[i].latches[1] = 0xffff;
	}
	
	ciaLastInterrupt = 0xffff;
}

void ciaWrite(unsigned char chip, unsigned char addr, unsigned char data) {
	switch (addr) {
		case 0x0:		// PDRa
			//ciaRegister[chip].PDRa = data;
			break;
		case 0x2:		// DDRa
			ciaRegister[chip].DDRa = data;
			break;
		case 0x3:		// DDRb
			ciaRegister[chip].DDRb = data;
			break;
		case 0x4:		// Timer A Low
			ciaTimers[chip].latches[0] &= 0xff00;
			ciaTimers[chip].latches[0] |= data;
			break;
		case 0x5:		// Timer A High
			{
			    int newTiming;

			    ciaTimers[chip].latches[0] &= 0x00ff;
			    ciaTimers[chip].latches[0] |= (data << 8);
			
			    newTiming = sh.hz / (ciaTimers[chip].latches[0] - 2);
			    if (chip) {
				if (ciaTimers[chip].enabled[0] == 1)
				    c64_start_freq_cia_a_nmi();
				
				c64_set_freq_cia_a_nmi(newTiming);
			    } else {
				if (ciaTimers[chip].enabled[0] == 1)
				    c64_start_freq_cia_a_irq();
				
				c64_set_freq_cia_a_irq(newTiming);
			    }
			    
			    // reload if stopped
			    //if (! (ciaTimers[chip].CR[0] & 0x1)) {
				//ciaTimers[chip].counters[0] = ciaTimers[chip].latches[0];
			    //}
			}
			break;
		case 0x6:		// Timer B Low
			ciaTimers[chip].latches[1] &= 0xff00;
			ciaTimers[chip].latches[1] |= data;
			break;
		case 0x7:		// Timer B High
			ciaTimers[chip].latches[1] &= 0x00ff;
			ciaTimers[chip].latches[1] |= (data << 8);
			// reload timer if stopped
			if (! (ciaTimers[chip].CR[1] & 0x1)) {
			    ciaTimers[chip].counters[1] = ciaTimers[chip].latches[1];
			}
			break;
		case 0xd:		// ICR
			if (data & 0x80) {
				ciaRegister[chip].ICR |= (data & 0x1f);
			} else {
				ciaRegister[chip].ICR &= ~data;
			}
			break;
		case 0xe:		// CRA
			ciaTimers[chip].CR[0] = data;
			
			if (data & 0x2) {
				c64_debug("Unsupported CIA Operation, TimerA -> PB6");
				exit(0);
			}
			
			if (data & 0x10) {
				ciaTimers[chip].CR[0] &= ~0x10;
				ciaTimers[chip].counters[0] = ciaTimers[chip].latches[0];
			}
			
			if ((data & 0x21) == 0x1) {
				ciaTimers[chip].enabled[0] = 1;
				if (chip) {
				    //int newTiming;

				    //newTiming = sh.hz / (ciaTimers[chip].latches[0] - 2);
				    //c64_set_freq_cia_a_nmi(newTiming);
				    c64_start_freq_cia_a_nmi();
				} else {
				    c64_start_freq_cia_a_irq();
				}
			} else {
				ciaTimers[chip].enabled[0] = 0;
			}
			break;
		case 0xf:		// CRB
			ciaTimers[chip].CR[1] = data;

			if (data & 0x2) {
				c64_debug("Unsupported CIA Operation, TimerB -> PB7");
				exit(0);
			}
			
			if (data & 0x10) {
				ciaTimers[chip].CR[1] &= ~0x10;
				ciaTimers[chip].counters[1] = ciaTimers[chip].latches[1];
			}

			if ((data & 0x61) == 0x1) {
				ciaTimers[chip].enabled[0] = 1;
			} else {
				ciaTimers[chip].enabled[0] = 0;
			}
			break;
		default:
			c64_debug("Unsupported CIA Write (%02x)\n", addr);
			exit(0);
	}
}

unsigned char ciaRead(unsigned char chip, unsigned char addr) {
	switch (addr) {
		case 0x0:		// PDRa
			return (ciaRegister[chip].PDRa | ~ciaRegister[chip].DDRa);
			break;
		case 0x1:		// PDRb
			{
				unsigned char data = (ciaRegister[chip].PDRb | ~ciaRegister[chip].DDRb);
				
				if ((ciaTimers[chip].CR[0] & 0x2) || (ciaTimers[chip].CR[1] & 0x2)) {
					c64_debug("CIA#%d: CRA %02x, CRB %02x\n", chip, ciaTimers[chip].CR[0], ciaTimers[chip].CR[1]);
					c64_debug("Unsupported CIA Operation\n");
					exit(0);
				}
				
				return data;                                                                                                                                                                                                                                                                          
				break;
			}
		case 0xd:		// ICR
			{
				unsigned char ret = ciaRegister[chip].IDR;
				ciaRegister[chip].IDR = 0;
				return ret;
			}
		case 0xe:
			return ciaTimers[chip].CR[0];
			break;
		default:
			c64_debug("Unsupported CIA Read (%02x)\n", addr);
			exit(0);
	}
}

/*
void ciaCheckTimers(void) {
	c64_debug("CIA#1: TimerA: %04x(%04x), TimerB: %04x(%04x)\n", ciaTimers[0].counters[0], ciaTimers[0].latches[0], ciaTimers[0].counters[1], ciaTimers[0].latches[1]);
	c64_debug("CIA#2: TimerA: %04x(%04x), TimerB: %04x(%04x)\n", ciaTimers[1].counters[0], ciaTimers[1].latches[0], ciaTimers[1].counters[1], ciaTimers[1].latches[1]);
	c64_debug("CIA#1: CRA: %02x, CRB: %02x, ICR: %02x, IDR: %02x\n", ciaTimers[0].CR[0], ciaTimers[0].CR[1], ciaRegister[0].ICR, ciaRegister[0].IDR);
	c64_debug("CIA#2: CRA: %02x, CRB: %02x, ICR: %02x, IDR: %02x\n", ciaTimers[1].CR[0], ciaTimers[1].CR[1], ciaRegister[1].ICR, ciaRegister[1].IDR);
	c64_debug("Enabled CIA: %d, %d, %d, %d\n", ciaTimers[0].enabled[0], ciaTimers[0].enabled[1], ciaTimers[1].enabled[0], ciaTimers[1].enabled[1]);
}
*/

// oppdaterer alle timere med time, lagrer chip og hvilken timer som timer ut
// i pekerne

/*
void ciaUpdateTimers(unsigned short time, unsigned char* chip, unsigned char* timer) {
	int i, j;
	// oppdater aktive tellere
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 2; j++) {
			if (ciaTimers[i].enabled[j]) {
				// sjekk om denne ble 0
				if (! (ciaTimers[i].counters[j] -= time)) {
					*chip = i;
					*timer = j;
				}
			}
		}
	}
}
*/

/*
void ciaUpdateTimerRegister(unsigned char chip, unsigned char timer) {
	// sjekk one-shot mode
	if (ciaTimers[chip].CR[timer] & 0x8) {
		ciaTimers[chip].CR[timer] &= ~0x1;
	}
	
	// reload timeout
	ciaTimers[chip].counters[timer] = ciaTimers[chip].latches[timer];

	// sett IDR porten, og sjekk om den aktuelle timer er satt til å generere interrupt
	ciaRegister[chip].IDR |= (1 << timer);
	if (ciaRegister[chip].IDR & ciaRegister[chip].ICR) {
		ciaRegister[chip].IDR |= (1 << 7);
	}

	// sjekk om timeren g�r for fort (sannsynligvis er det da samples)
	if (ciaTimers[chip].latches[timer] < CIA_INTERRUPT_THRESHOLD)
		ciaTimers[chip].enabled[timer] = 0;
}

*/

