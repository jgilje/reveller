#include "platform-support-common.h"
#include "platform-support.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef unix
#include <unistd.h>
#endif

#ifdef _WIN32
#include <Windows.h>
#endif

static void dummy_usleep(uint32_t us) {
#if defined unix
  usleep(us);
#elif defined _WIN32
  Sleep(us / 1000);
#endif
}

static void dummy_sid_write(uint8_t reg, uint8_t data) {
  //	platform_debug("SID Write: %x: %x\n", reg, data);
}

static void dummy_power() {}
static void dummy_shutdown(uint32_t state) {}

static void init() {}

struct reveller_platform dummy_platform = {
    .init = &init,

    .debug = &common_platform_debug,
    .abort = &common_platform_abort,

    .pause = &common_pause,
    .resume = &common_resume,
    .usleep = &dummy_usleep,
    .power = &dummy_power,
    .shutdown = &dummy_shutdown,

    .read = &common_platform_read_source,

    .sid_block_start = &common_sid_block_start,
    .sid_block_end = &common_sid_block_end,
    .sid_write = &dummy_sid_write,

    .platform_id = "Dummy",
};
