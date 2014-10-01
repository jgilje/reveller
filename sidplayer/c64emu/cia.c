// CIA (MOS6526) emulation
// mainly dealing with the timers

#include "cia.h"
#include "platform-support.h"

int32_t c64_cia_next_timer(void) {
	int32_t next = INT32_MAX;
	// uint32_t next_chip, next_timer;
	int i, j;
	
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 2; j++) {
			if (ciaTimers[i].enabled[j]) {
				if (ciaTimers[i].counters[j] < next) {
					next = ciaTimers[i].counters[j];
					/*
					next_chip = i;
					next_timer = j;
					*/
				}
			}
		}
	}
	
	/*
	if (next > 0) {
		platform_debug("c64_cia_next_irq(): from %d-%d at %d\n", next_chip, next_timer, next);
	}
	*/
	
	if (next == INT32_MAX) {
		return -1;
	}
	return next;
}

void c64_cia_update_timers(int32_t next) {
	int i, j;
	
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 2; j++) {
			if (ciaTimers[i].enabled[j]) {
				ciaTimers[i].counters[j] -= next;
				if (ciaTimers[i].counters[j] == 0) {
					if (ciaTimers[i].interrupt_enabled[j]) {
						ciaTimers[i].interrupt_triggered[j] = 1;
						ciaRegister[i].IDR |= (1 << j);
					}
					ciaTimers[i].counters[j] = ciaTimers[i].latches[j];
					if (ciaTimers[i].oneshot[j]) {
						ciaTimers[i].enabled[j] = 0;
					}
				}
			}
		}
	}	
}

uint32_t c64_cia_interrupt(uint32_t chip) {
	int i;
	uint32_t ret = 0;
	
	for (i = 0; i < 2; i++) {
		if (ciaTimers[chip].interrupt_triggered[i]) {
			ciaTimers[chip].interrupt_triggered[i] = 0;
			ret = 1;
		}
	}
	
	return ret;
}

uint32_t c64_cia_nmi(void) {
	return c64_cia_interrupt(1);
}

uint32_t c64_cia_irq(void) {
	return c64_cia_interrupt(0);
}

void c64_cia_init() {
	int i;
	memset(&ciaRegister, 0, sizeof(ciaRegister));
	memset(&ciaTimers, 0, sizeof(ciaTimers));
	
	for (i = 0; i < 2; i++) {
		ciaTimers[i].interrupt_enabled[0] = 0;
		ciaTimers[i].interrupt_enabled[1] = 0;
		ciaTimers[i].interrupt_triggered[0] = 0;
		ciaTimers[i].interrupt_triggered[1] = 0;
		ciaTimers[i].counters[0] = 0xffff;
		ciaTimers[i].counters[1] = 0xffff;
		ciaTimers[i].latches[0] = 0xffff;
		ciaTimers[i].latches[1] = 0xffff;
		ciaTimers[i].enabled[0] = 0;
		ciaTimers[i].enabled[1] = 0;
		ciaTimers[i].CR[0] = 0;
		ciaTimers[i].CR[1] = 0;
	}
}

// Write to CIA control register
void c64_cia_write_cr(unsigned char chip, unsigned char data, unsigned char timer_char) {
	// TODO does not handle timerB correctly yet
	unsigned char timer_no = timer_char == 'A' ? 0 : 1;
	ciaTimers[chip].CR[timer_no] = data;
	
	if (data & 0x1) {
		ciaTimers[chip].enabled[timer_no] = 1;
	} else {
		ciaTimers[chip].enabled[timer_no] = 0;
	}
	if (data & 0x2) {
		platform_debug("WARNING: CIA%d-%c: Unsupported CIA Operation, TimerA -> PB6, ignored\n", chip, timer_char);
	}
	if (data & 0x4) {
		platform_debug("WARNING: CIA%d-%c: Unsupported CIA Operation, TimerA -> Toggle output\n", chip, timer_char);
	}
	if (data & 0x8) {
		ciaTimers[chip].oneshot[timer_no] = 1;
	} else {
		ciaTimers[chip].oneshot[timer_no] = 0;
	}
	if (data & 0x10) {
		ciaTimers[chip].CR[timer_no] &= ~0x10;
		ciaTimers[chip].counters[timer_no] = ciaTimers[chip].latches[timer_no];
	}
	if (data & 0x20) {
		platform_debug("WARNING: Unsupported CIA Operation, TimerA -> Count ext. events\n");
	}
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
		    ciaTimers[chip].latches[0] &= 0x00ff;
		    ciaTimers[chip].latches[0] |= (data << 8);
		    
		    // reload timer if stopped
			if (! (ciaTimers[chip].CR[0] & 0x1)) {
				ciaTimers[chip].counters[0] = ciaTimers[chip].latches[0];
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
		case 0xc:		// Serial shift register
			ciaRegister[chip].SDR = data;
			break;
		case 0xd:		// ICR
			if (data & 0x80) {
				if (data & 0x1) {
					ciaTimers[chip].interrupt_enabled[0] = 1;
				}
				if (data & 0x2) {
					ciaTimers[chip].interrupt_enabled[1] = 1;
				}
				ciaRegister[chip].ICR |= (data & 0x1f);
			} else {
				if (data & 0x1) {
					ciaTimers[chip].interrupt_enabled[0] = 0;
				}
				if (data & 0x2) {
					ciaTimers[chip].interrupt_enabled[1] = 0;
				}
				ciaRegister[chip].ICR &= ~data;
			}
			break;
		case 0xe:		// CRA
			c64_cia_write_cr(chip, data, 'A');
			break;
		case 0xf:		// CRB
			c64_cia_write_cr(chip, data, 'B');
			break;
		default:
			platform_abort("Unsupported CIA Write on chip %d: (%02x)\n", chip, addr);
	}
}

unsigned char ciaRead(unsigned char chip, unsigned char addr) {
#ifdef DEBUG
	platform_debug(" (CIA read from chip %d %x) ", chip, addr);
#endif
	switch (addr) {
		case 0x0:		// PDRa
			if (! chip) {
				return 0xff;
			}
			return 0xd0;
			break;
		case 0x1:		// PDRb
			if (! chip) {	
				return 0xff;
			}
			
			platform_abort("CIA#2 PDRb is not emulated\n");
			
			/*
			{
				unsigned char data = (ciaRegister[chip].PDRb | ~ciaRegister[chip].DDRb);
				
				if ((ciaTimers[chip].CR[0] & 0x2) || (ciaTimers[chip].CR[1] & 0x2)) {
					platform_debug("CIA#%d: CRA %02x, CRB %02x\n", chip, ciaTimers[chip].CR[0], ciaTimers[chip].CR[1]);
					platform_debug("Unsupported CIA Operation\n");
					exit(0);
				}
				
				printf("CIA RET READa (%x %x): %x\n", ciaRegister[chip].PDRb, ciaRegister[chip].DDRb, data);
				return data;
				break;
			}
			*/
		case 0x4:		// timerA low
			return ciaTimers[chip].counters[0] & 0xff;
			break;
		case 0x5:		// timerA high
			return (ciaTimers[chip].counters[0] >> 8) & 0xff;
			break;
		case 0xc:
			return ciaRegister[chip].SDR;
			break;
		case 0xd:		// IDR
			{
				unsigned char ret = ciaRegister[chip].IDR;
				if (ret) {
					ret |= 0x80;
				}
				ciaRegister[chip].IDR = 0;
#ifdef DEBUG
				platform_debug("ciaRead(): read from chip %d, addr: 0xd, returning %x\n", chip, ret);
#endif
				return ret;
			}
		case 0xe:
#ifdef DEBUG
			platform_debug("ciaRead(): read from chip %d, addr: 0xe, returning %x\n", chip, ciaTimers[chip].CR[0]);
#endif
			return ciaTimers[chip].CR[0];
			break;
		default:
			platform_abort("Unsupported CIA Read on chip %d: (%02x)\n", chip, addr);
	}
	
	platform_abort("Unsupported CIA Read (%02x)\n", addr);
	return 0x0;
}

