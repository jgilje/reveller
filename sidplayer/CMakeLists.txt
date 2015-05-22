cmake_minimum_required(VERSION 2.8)

PROJECT(sidplayer)

ADD_SUBDIRECTORY(c64emu) # "${CMAKE_CURRENT_BINARY_DIR}/c64emu")

SET(sidplayer-hammer_SOURCES
    sidplayer-hammer.c
    lcd-hammer.c
    console-interface.c
    c64emu-support-common.c
    platform-support-hammer.c
    common/endian-conversion.c
)

SET(sidplayer-hammer-standalone_SOURCES
    sidplayer-hammer-standalone.c
    lcd-hammer.c
    c64emu-support-common.c
    platform-support-hammer.c
    common/endian-conversion.c
    common/insane-menu.c
)

SET(sidplayer-rpi_SOURCES
    sidplayer-rpi.c
    c64emu-support-common.c
    platform-support-rpi.c
    console-interface.c
    common/endian-conversion.c
)

SET(sidplayer-bbb_SOURCES
    sidplayer-bbb.c
    c64emu-support-common.c
    platform-support-bbb.c
    console-interface.c
    common/endian-conversion.c
)

SET(sidplayer-dummy_SOURCES
    sidplayer-dummy.c
    c64emu-support-common.c
    platform-support-dummy.c
    console-interface.c
    common/endian-conversion.c
)

# ADD_EXECUTABLE(sidplayer-hammer
#     ${sidplayer-hammer_SOURCES}
# )
# ADD_EXECUTABLE(sidplayer-hammer-standalone
#     ${sidplayer-hammer-standalone_SOURCES}
# )
# ADD_EXECUTABLE(sidplayer-rpi
#     ${sidplayer-rpi_SOURCES}
# )
# ADD_EXECUTABLE(sidplayer-bbb
#     ${sidplayer-bbb_SOURCES}
# )
ADD_EXECUTABLE(sidplayer-dummy
    ${sidplayer-dummy_SOURCES}
)

include_directories(c64emu common)

set(CMAKE_C_FLAGS "-Wall")

# TARGET_LINK_LIBRARIES(sidplayer-hammer c64emu)
# TARGET_LINK_LIBRARIES(sidplayer-hammer-standalone c64emu)
# TARGET_LINK_LIBRARIES(sidplayer-rpi c64emu)
# TARGET_LINK_LIBRARIES(sidplayer-bbb c64emu)
TARGET_LINK_LIBRARIES(sidplayer-dummy c64emu)

