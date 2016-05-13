#ifndef PLATFORM_SUPPORT
#define PLATFORM_SUPPORT

#include <stdint.h>
#include <stddef.h>

/*
  These functions must be implemented to support a new platform
*/
void platform_debug(const char *msg, ...);
void platform_abort(const char *msg, ...);

void platform_usleep(uint32_t us);
void platform_shutdown();

size_t c64_read_source(uint32_t offset, uint32_t length, uint8_t *dest);

void c64_sid_block_start(void);
void c64_sid_block_end(void);

void c64_sid_write(uint8_t reg, uint8_t data);

void c64_set_freq_vic(uint32_t hz);
void c64_start_freq_vic(void);

#endif
