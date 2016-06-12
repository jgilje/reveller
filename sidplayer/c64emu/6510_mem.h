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

extern unsigned char *pages[256];	// pages, inneholder pekere til aktuelle minneomrÂder
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
void c64_initMem(void);
void c64_resetMem(void);
void c64_dumpMem(void);

void c64_storeMemRAMChar(unsigned short addr, unsigned char data);
void c64_storeMemRAMShort(unsigned short addr, unsigned char data1, unsigned char data2);
void c64_loadMem(unsigned short addr);
void c64_storeMem(unsigned char data);
void c64_fetchOP(void);

void c64_memImm(void);
void c64_memAbsoluteAddr(void);
void c64_memAbsoluteAddrX(void);
void c64_memAbsoluteAddrY(void);
void c64_memZero(void);
void c64_memZeroX(void);
void c64_memZeroY(void);
void c64_memIndirectZeroX(void);
void c64_memIndirectZeroY(void);
void c64_memStack(void);

#endif
