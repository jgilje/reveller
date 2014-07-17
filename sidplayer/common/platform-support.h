#ifndef PLATFORM_SUPPORT
#define PLATFORM_SUPPORT

#include <stdint.h>

/*
  These functions must be implemented to support a new platform
*/
void c64_debug(char *msg, ...);

void platform_usleep(uint32_t us);
size_t c64_read_source(uint32_t offset, uint32_t length, uint8_t *dest);

void c64_sid_block_start(void);
void c64_sid_block_end(void);

void c64_sid_write(uint8_t reg, uint8_t data);

void c64_set_freq_vic(uint32_t hz);
void c64_start_freq_vic(void);

#endif
