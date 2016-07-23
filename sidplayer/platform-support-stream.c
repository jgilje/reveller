#include "platform-support.h"
#include "platform-support-common.h"

#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096

static int reveller_fileno = -1;
static uint8_t buffer[BUFFER_SIZE];
static int buffer_pos = 0;

static void advance(int bytes) {
    buffer_pos += bytes;

    if (buffer_pos >= (BUFFER_SIZE - 4)) {
        write(reveller_fileno, buffer, buffer_pos);
        buffer_pos = 0;
    }
}

static void stream_usleep(uint32_t us) {
    buffer[buffer_pos + 0] = 0x20;
    buffer[buffer_pos + 1] = (us >> 16) & 0xff;
    buffer[buffer_pos + 2] = (us >>  8) & 0xff;
    buffer[buffer_pos + 3] = us & 0xff;
    advance(4);
}

static void stream_shutdown() {
    if (reveller_fileno > 0) {
        close(reveller_fileno);
    }
}

static void stream_sid_write(uint8_t reg, uint8_t data) {
    buffer[buffer_pos + 0] = reg & 0x1f;
    buffer[buffer_pos + 1] = data;
    advance(2);
}

static void init() {
    reveller_fileno = open("/dev/reveller", O_WRONLY);
    if (reveller_fileno < 0) {
        fprintf(stderr, "Failed to open /dev/reveller\n");
        exit(1);
    }
}

struct reveller_platform stream_platform = {
    .init = &init,

    .debug = &common_platform_debug,
    .abort = &common_platform_abort,

    .usleep = &stream_usleep,
    .shutdown = &stream_shutdown,

    .read = &common_platform_read_source,

    .sid_block_start = &common_sid_block_start,
    .sid_block_end = &common_sid_block_end,
    .sid_write = &stream_sid_write,

    .platform_id = "Stream",
};
