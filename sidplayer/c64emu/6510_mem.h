#ifndef _6510_MEM_H_
#define _6510_MEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "6510.h"
// #include "sidheader.h"
// #include "sid.h"
// #include "helper.h"

// Ingen page cross
//#define PC_OP *(pages[reg.page] + reg.offset)
//#define PC_OP1 *(pages[reg.page] + reg.offset + 1)
//#define PC_OP2 *(pages[reg.page] + reg.offset + 2)
// For evt. page cross (PC_OP_B(hvilken byte krysser)
//#define PC_OP_B1 *(pages[reg.page + 1])
//#define PC_OP_B2 *(pages[reg.page + 1] + 1)

unsigned char *pages[256];	// pages, inneholder pekere til aktuelle minneomrÂder
// unsigned char *rompages[256];	// pages, inneholder pekere til aktuelle rom områder

//void (*fetchOP)();
//void (*loadMem)(unsigned short addr);
//void (*storeMem)(unsigned char addr);

// vi lar storeMem avgj¯re, istedet, ATM
//unsigned char *write_pages[256];	// skrivetabell, hvilken funksjon hÂndterer skriving til en page

//static unsigned char chararcter[] = { 
//	#include "char.bin" 
//};
//static unsigned char basic[] = { 
//	#include "basic.bin" 
//};

// signaturer
void createMem(unsigned char page);
void initMem();
void resetMem(void);
void dumpMem(void);

void storeMemRAMChar(unsigned short addr, unsigned char data);
void storeMemRAMShort(unsigned short addr, unsigned char data1, unsigned char data2);
void loadMemRAM(unsigned char page, unsigned char offset);
void loadMem(unsigned short addr);
void storeMem(unsigned char data);
void fetchOP(void);

void memImm(void);
void memAbsoluteAddr(void);
void memAbsoluteAddrX(void);
void memAbsoluteAddrY(void);
void memZero(void);
void memZeroX(void);
void memZeroY(void);
void memIndirectZeroX(void);
void memIndirectZeroY(void);
void memStack(void);

#endif
