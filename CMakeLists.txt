set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

cmake_policy(SET CMP0048 NEW)
PROJECT(reveller VERSION 0.1.0 LANGUAGES C CXX Go)

include(cmake/golang.cmake)

ADD_SUBDIRECTORY(sidplayer)
ADD_SUBDIRECTORY(sidplayer-web-backend)


install(TARGETS sidplayer)
# sidplayer-web-backend already have an install def.
install(DIRECTORY sidplayer-web-frontend/ DESTINATION www/)

set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/reveller")
set(CPACK_PACKAGE_VENDOR "FluxXx")
set(CPACK_PACKAGE_CONTACT "jgilje@jgilje.net")

set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_RPM_FILE_NAME "RPM-DEFAULT")

set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${CMAKE_CURRENT_SOURCE_DIR}/debian/prerm"
    "${CMAKE_CURRENT_SOURCE_DIR}/debian/postinst"
    "${CMAKE_CURRENT_SOURCE_DIR}/debian/postrm"
)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/sidplayer-web-backend/systemd/reveller.service
  DESTINATION /usr/local/lib/systemd/system
)

INSTALL(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/sidplayer/kernel-driver/rpi/Makefile
  ${CMAKE_CURRENT_SOURCE_DIR}/sidplayer/kernel-driver/rpi/reveller.c
  ${CMAKE_CURRENT_SOURCE_DIR}/sidplayer/kernel-driver/rpi/dkms.conf
  DESTINATION /usr/src/reveller-0.1.0
)

include(CPack)
