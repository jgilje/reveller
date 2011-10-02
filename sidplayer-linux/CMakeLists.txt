cmake_minimum_required(VERSION 2.8)

PROJECT(sidplayer)

ADD_SUBDIRECTORY(../c64emu "${CMAKE_CURRENT_BINARY_DIR}/c64emu")

SET(sidplayer-linux_SOURCES
    sidplayer-linux.c
    c64emu-support-common.c
    c64emu-support-gpio.c
    ../helpers/endian-conversion.c
)

ADD_EXECUTABLE(sidplayer-linux
    ${sidplayer-linux_SOURCES}
)

include_directories(../c64emu)

TARGET_LINK_LIBRARIES(sidplayer-linux c64emu)

