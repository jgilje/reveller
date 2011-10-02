cmake_minimum_required(VERSION 2.8)

PROJECT(c64emu)

SET(c64emu_SOURCES
    6510.c 6510_mem.c vic.c cia.c sidheader.c
)

ADD_LIBRARY(c64emu
    ${c64emu_SOURCES}
)

