#ifndef _VIC_H_
#define _VIC_H_

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
// #include "rprintf.h"
// #include "sid.h"

typedef struct vicRegister {
	unsigned char icr;
	unsigned char idr;
} vicRegister;

vicRegister vicReg;
unsigned char vicInterrupt;

void vicInit(void);
void vicWrite(unsigned char addr, unsigned char data);
unsigned char vicRead(unsigned char addr);

#endif
