cmake_minimum_required(VERSION 2.8)

PROJECT(c64emu)

SET(c64emu_SOURCES
    6510.c 6510_mem.c vic.c cia.c sidheader.c
)

SET(CMAKE_C_FLAGS "-Wall")
INCLUDE_DIRECTORIES(../helpers ../common)

ADD_LIBRARY(c64emu
    ${c64emu_SOURCES}
)

