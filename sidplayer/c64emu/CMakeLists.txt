cmake_minimum_required(VERSION 3.10)

SET(CMAKE_C_FLAGS "-Wall")
INCLUDE_DIRECTORIES(../helpers ../common)

ADD_LIBRARY(c64emu
    6510.c
    6510_mem.c
    vic.c
    cia.c
    sidheader.c
)

