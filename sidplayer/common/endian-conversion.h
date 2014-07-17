#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#include <stdio.h>

#ifdef sgi
#define __BIG_ENDIAN__
#endif

#ifdef hppa
#define __BIG_ENDIAN__
#endif

#ifdef sparc
#define __BIG_ENDIAN__
#endif

/*
#ifdef __arm__
#define __BIG_ENDIAN__
#endif
*/

void read_bigEndian_int(int* dest, unsigned char* source);
void read_bigEndian_short(short* dest, unsigned char* source);
void read_bigEndian_ushort(unsigned short* dest, unsigned char* source);
void read_littleEndian_short(short* dest, unsigned char* source);
void read_littleEndian_ushort(unsigned short* dest, unsigned char* source);

#endif
