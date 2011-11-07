#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

extern FILE* inputSidFile;
extern FILE* outfile;

void c64_sid_block_start(void) {
    fprintf(outfile, "s");
}

void c64_sid_block_end(void) {
    fprintf(outfile, "e");
}

void c64_sid_write(uint8_t reg, uint8_t data) {
    fprintf(outfile, "w%c%c", reg, data);
}

void c64_set_freq_vic(uint32_t hz) {
    fprintf(outfile, "t2%c%c", (hz >> 8) & 0x000000ff, hz & 0x000000ff);
}
void c64_start_freq_vic(void) {
    fprintf(outfile, "T2");
}

void c64_set_freq_cia_a_irq(uint32_t hz) {
    fprintf(outfile, "t1%c%c", (hz >> 8) & 0x000000ff, hz & 0x000000ff);
}
void c64_start_freq_cia_a_irq(void) {
    fprintf(outfile, "T1");
}

void c64_set_freq_cia_a_nmi(uint32_t hz) {
    fprintf(outfile, "t0%c%c", (hz >> 8) & 0x000000ff, hz & 0x000000ff);
}
void c64_start_freq_cia_a_nmi(void) {
    fprintf(outfile, "T2");
}

