#ifndef _VIC_H_
#define _VIC_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef struct vicRegister {
	unsigned char icr;
	unsigned char idr;
} vicRegister;
vicRegister vicReg;

typedef struct c64_vic_timer_t {
	unsigned short latch;
	unsigned short counter;
	unsigned char enabled;
	unsigned char interrupt;
} c64_vic_timer_t;
c64_vic_timer_t c64_vic_timer;

void vicWrite(unsigned char addr, unsigned char data);
unsigned char vicRead(unsigned char addr);

void c64_vic_init(void);
int32_t c64_vic_next_timer(void);
void c64_vic_update_timer(int32_t next);
uint32_t c64_vic_irq(void);

#endif
