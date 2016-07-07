#include "platform-support.h"
#include "platform-support-common.h"

#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static int reveller_fileno = -1;

union command {
    uint8_t c[4];
    uint32_t us;
};

static void stream_usleep(uint32_t us) {
    // union command u = {.us = us & 0xffffff};
    uint8_t c[4];
    c[0] = 0x20;
    c[1] = (us >> 16) & 0xff;
    c[2] = (us >>  8) & 0xff;
    c[3] = us & 0xff;
    write(reveller_fileno, &c, sizeof(c));
}

static void stream_shutdown() {
    if (reveller_fileno > 0) {
        close(reveller_fileno);
    }
}

static void stream_sid_write(uint8_t reg, uint8_t data) {
    uint8_t buf[2];
    buf[0] = reg & 0x1f;
    buf[1] = data;
    write(reveller_fileno, buf, 2);
}

static void init() {
    reveller_fileno = open("/dev/reveller", O_WRONLY);
    if (reveller_fileno < 0) {
        fprintf(stderr, "Failed to open /dev/reveller\n");
        exit(1);
    }
}

static void stream_platform_debug(const char *msg, ...) {
}

void stream_platform_abort(const char *msg, ...) {
    exit(1);
}

struct reveller_platform stream_platform = {
    .init = &init,

    .debug = &stream_platform_debug,
    .abort = &stream_platform_abort,

    .usleep = &stream_usleep,
    .shutdown = &stream_shutdown,

    .read = &common_platform_read_source,

    .sid_block_start = &common_sid_block_start,
    .sid_block_end = &common_sid_block_end,
    .sid_write = &stream_sid_write,

    .platform_id = "Stream",
};
