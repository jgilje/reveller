CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(sidplayer)

ADD_SUBDIRECTORY(c64emu) # "${CMAKE_CURRENT_BINARY_DIR}/c64emu")

ADD_LIBRARY(reveller
    console-interface.c

    platform-support-common.c
    platform-support-rpi.c
    platform-support-bbb.c
    platform-support-dummy.c
    platform-support-stream.c

    common/endian-conversion.c
)

ADD_EXECUTABLE(sidplayer
    sidplayer.c
)

INCLUDE_DIRECTORIES(c64emu common)

SET(CMAKE_C_FLAGS "-Wall")

TARGET_LINK_LIBRARIES(sidplayer c64emu reveller)

