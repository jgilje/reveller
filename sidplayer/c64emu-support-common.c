#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

extern FILE* inputSidFile;
extern FILE* outfile;

void platform_debug(const char *msg, ...) {
    va_list argp;
    
    va_start(argp, msg);
    vfprintf(stdout, msg, argp);
    va_end(argp);
    
    fflush(stdout);
}

void platform_abort(char *msg, ...) {
    va_list argp;
    
    va_start(argp, msg);
    vfprintf(stderr, msg, argp);
    va_end(argp);
    
    fflush(stdout);
    fflush(stderr);
    
    exit(1);
}

size_t c64_read_source(uint32_t offset, uint32_t length, uint8_t *dest) {
    size_t ret;
    // printf("Reading at offset %d - %d bytes\n", offset, length);
    if (fseek(inputSidFile, offset, SEEK_SET) < 0) {
        platform_abort("Failed to seek inputSidFile, offset %x\n", offset);
    }
    ret = fread(dest, 1, length, inputSidFile);
    return ret;
}
