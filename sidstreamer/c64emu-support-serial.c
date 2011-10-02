#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

extern FILE* inputSidFile;
extern int tty_fd;

void c64_sid_block_start(void) {
    write(tty_fd, "s", 1);
}

void c64_sid_block_end(void) {
    write(tty_fd, "e", 1);
}

void c64_sid_write(uint8_t reg, uint8_t data) {
    char b[3];
    sprintf(b, "w%c%c", reg, data);
    write(tty_fd, b, 3);
}

void c64_set_freq_vic(uint32_t hz) {
    char b[4];
    sprintf(b, "t2%c%c", (hz >> 8) & 0x000000ff, hz & 0x000000ff);
    write(tty_fd, b, 4);
}
void c64_start_freq_vic(void) {
    write(tty_fd, "T2", 2);
}

void c64_set_freq_cia_a_irq(uint32_t hz) {
    char b[4];
    sprintf(b, "t1%c%c", (hz >> 8) & 0x000000ff, hz & 0x000000ff);
    write(tty_fd, b, 4);
}
void c64_start_freq_cia_a_irq(void) {
    write(tty_fd, "T1", 2);
}

void c64_set_freq_cia_a_nmi(uint32_t hz) {
    char b[4];
    sprintf(b, "t0%c%c", (hz >> 8) & 0x000000ff, hz & 0x000000ff);
    fprintf(stderr, "Exit at %s: %d\n", __FILE__, __LINE__);
    exit(1);
}
void c64_start_freq_cia_a_nmi(void) {
    fprintf(stderr, "Exit at %s: %d\n", __FILE__, __LINE__);
    exit(1);
}
