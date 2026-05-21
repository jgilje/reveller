#ifndef PLATFORM_SUPPORT_DUMMY_H
#define PLATFORM_SUPPORT_DUMMY_H

#include <stdint.h>

extern struct reveller_platform dummy_platform;

uint8_t dummy_sid_read(uint8_t reg);

#endif
