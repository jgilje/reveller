cmake_minimum_required(VERSION 2.8)

PROJECT(sidplayer)

ADD_SUBDIRECTORY(../c64emu "${CMAKE_CURRENT_BINARY_DIR}/c64emu")

SET(sidplayer-linux_SOURCES
    lcd.c
    sidplayer-linux.c
    c64emu-support-common.c
    c64emu-support-gpio-hammer.c
    ../helpers/endian-conversion.c
)

SET(sidplayer-hammer-gpio-linux_SOURCES
    lcd.c
    sidplayer-hammer-gpio-linux.c
    c64emu-support-common.c
    c64emu-support-gpio-hammer.c
    ../helpers/endian-conversion.c
    ../helpers/insane-menu.c
)

SET(sidplayer-rpi-gpio-linux_SOURCES
    sidplayer-rpi-gpio-linux.c
    c64emu-support-common.c
    c64emu-support-gpio-rpi.c
    ../helpers/endian-conversion.c
)

ADD_EXECUTABLE(sidplayer-linux
    ${sidplayer-linux_SOURCES}
)
ADD_EXECUTABLE(sidplayer-hammer-gpio-linux
    ${sidplayer-hammer-gpio-linux_SOURCES}
)
ADD_EXECUTABLE(sidplayer-rpi-gpio-linux
    ${sidplayer-rpi-gpio-linux_SOURCES}
)

include_directories(../c64emu ../helpers)

set(CMAKE_C_FLAGS "-Wall")

TARGET_LINK_LIBRARIES(sidplayer-linux c64emu rt)
TARGET_LINK_LIBRARIES(sidplayer-hammer-gpio-linux c64emu rt)
TARGET_LINK_LIBRARIES(sidplayer-rpi-gpio-linux c64emu rt)

