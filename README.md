# Reveller

Connect your SID-file (6581/8580) to a Raspberry Pi, BeagleBone Black or TinCan Tools Hammer - 
and listen to the beautiful un-emulated SID-music.

## Compiling
[sidplayer/](https://github.com/jgilje/reveller/tree/master/sidplayer) contains the core program. It emulates a simple C=64,
and redirects SID-IO to the chip. Build using CMake.

[sidplayer-web/](https://github.com/jgilje/reveller/tree/master/sidplayer-web) is a frontend for the core, allowing
clients to connect and control the more basic core. This is a Go project.

[qt-gui/](https://github.com/jgilje/reveller/tree/master/qt-gui) is a client for the frontend written in Qt. Build
using qmake.

## Schematics
Complete Eagle board layout and schematics is available for the Raspberry Pi 
 ([board](https://raw.githubusercontent.com/jgilje/reveller/master/docs/reveller-pi.brd),
  [schematics](https://raw.githubusercontent.com/jgilje/reveller/master/docs/reveller-pi.sch))
port. Schematics for the other ports are sadly undocumented for the moment, although the core has been ported
to the other platforms.

## Cases
* BeagleBone Black, available at http://www.thingiverse.com/thing:1019636
