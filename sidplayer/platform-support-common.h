#ifndef PLATFORM_SUPPORT_COMMON_H
#define PLATFORM_SUPPORT_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

void common_platform_debug(const char *msg, ...);
void common_platform_abort(const char *msg, ...);
size_t common_platform_read_source(uint32_t offset, uint32_t length, uint8_t *dest);

void common_sid_block_start(void);
void common_sid_block_end(void);

int open_mem();
void* get_addr(uint32_t addr);
void release_addr(void* addr);

void set_realtime();
void detect_platform();

#endif
