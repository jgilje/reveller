#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

extern FILE* inputSidFile;
extern FILE* outfile;

void c64_debug(char *msg, ...) {
    va_list argp;
    
    va_start(argp, msg);
    vfprintf(stdout, msg, argp);
    va_end(argp);
}

size_t c64_read_source(uint32_t offset, uint32_t length, uint8_t *dest) {
    size_t ret;
    // printf("Reading at offset %d - %d bytes\n", offset, length);
    fseek(inputSidFile, offset, SEEK_SET);
    ret = fread(dest, 1, length, inputSidFile);
    return ret;
}
