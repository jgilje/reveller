#ifndef PLATFORM_SUPPORT_H
#define PLATFORM_SUPPORT_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

/*
  These functions must be implemented to support a new platform
*/
struct reveller_platform {
    void (*init)();

    void (*debug)(const char *msg, ...);
    void (*abort)(const char *msg, ...);

    void (*pause)();
    void (*resume)();
    void (*usleep)(uint32_t us);
    void (*power)(uint32_t state);
    void (*shutdown)();

    size_t (*read)(uint32_t offset, uint32_t length, uint8_t *dest);
    void (*flush)();
    int (*write_handle)();

    void (*sid_block_start)(void);
    void (*sid_block_end)(void);
    void (*sid_write)(uint8_t reg, uint8_t data);
    uint8_t (*sid_read)(uint8_t reg);

    const char* platform_id;
};

extern struct reveller_platform *reveller;
extern FILE* reveller_input_file;

typedef enum sidchip_implementation {
    SIDCHIP_UNKNOWN,
    SIDCHIP_PDSID,
} sidchip_implementation_t;
extern sidchip_implementation_t sidchip_implementation;

typedef enum sidchip {
    MOS6581,
    MOS8580,
} sidchip_t;
void set_chipmode(sidchip_t sidchip);
extern int sidchip_override;

#endif
