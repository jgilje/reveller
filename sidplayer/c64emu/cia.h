#ifndef _CIA_H_
#define _CIA_H_

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// #include "rprintf.h"

#include "6510.h"
#include "sidheader.h"

typedef struct ciaReg {
	// Peripheral Data Register
	unsigned char PDRa;
	unsigned char PDRb;
	// Data Direction Register
	unsigned char DDRa;
	unsigned char DDRb;
	// Timer low, high
//	unsigned char TimerAl;
//	unsigned char TimerAh;
//	unsigned char TimerBl;
//	unsigned char TimerBh;
	// Time Of Day, 1/10s, s, m, h
	unsigned char TODts;
	unsigned char TODs;
	unsigned char TODm;
	unsigned char TODh;
	// Serial Data Register
	unsigned char SDR;
	// Interrupt Control Register
	unsigned char ICR;	// write
	unsigned char IDR;	// read
} ciaReg;

typedef struct ciaTimer {
	unsigned short latches[2];
	unsigned short counters[2];
	unsigned char enabled[2];
	unsigned char oneshot[2];
	unsigned char interrupt_enabled[2];
	unsigned char interrupt_triggered[2];
	unsigned char CR[2];		// control registers
} ciaTimer;

//#define CIA_INTERRUPT_THRESHOLD 0x2000

void c64_cia_init(void);

unsigned char c64_cia_read(unsigned char chip, unsigned char addr);
void c64_cia_write(unsigned char chip, unsigned char addr, unsigned char data);

int32_t c64_cia_next_timer(void);
void c64_cia_update_timers(int32_t next);
uint32_t c64_cia_nmi(void);
uint32_t c64_cia_irq(void);

#endif
