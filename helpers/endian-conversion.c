#include "endian.h"

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
void read_bigEndian_int(int *dest, unsigned char* source) {
    *dest = ((source[0] << 24 | source[1] << 16 | source[2] << 8 | source[3]));
}

void read_bigEndian_short(short *dest, unsigned char* source) {
    *dest = (source[0] << 8);
    *dest |= source[1];
}

void read_littleEndian_short(short *dest, unsigned char* source) {
    *dest = * (short*) source;
}
#endif
