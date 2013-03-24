cmake_minimum_required(VERSION 2.8)

PROJECT(sidstreamer)

ADD_SUBDIRECTORY(../c64emu "${CMAKE_CURRENT_BINARY_DIR}/c64emu")

SET(sidstreamer-file_SOURCES
    sidstreamer-file.c
    c64emu-support-common.c
    c64emu-support-file.c
    ../helpers/endian-conversion.c
)

SET(sidstreamer-serial_SOURCES
    sidstreamer-serial.c
    c64emu-support-common.c
    c64emu-support-serial.c
    ../helpers/endian-conversion.c
)

ADD_EXECUTABLE(sidstreamer-file
    ${sidstreamer-file_SOURCES}
)
ADD_EXECUTABLE(sidstreamer-serial
    ${sidstreamer-serial_SOURCES}
)

include_directories(../c64emu)

set(CMAKE_C_FLAGS "-Wall -Wextra")

TARGET_LINK_LIBRARIES(sidstreamer-file c64emu)
TARGET_LINK_LIBRARIES(sidstreamer-serial c64emu)

