#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sidheader.h"
#include "platform-support.h"
#include "endian-conversion.h"

int parseHeader() {
    unsigned char buffer[128];
    c64_read_source(0, 128, buffer);
    // sett filpeker til 0
    //fseek(sidFile, 0, SEEK_SET);

    // nullstill sh
    memset(&sh, 0, sizeof(sidHeader));

    // Les header
    //fread(&sh.type, sizeof(char), 4, sidFile);
    memcpy(&sh.type, buffer, 4);
    sh.type[4] = 0x0;

    if (strcmp(sh.type, "RSID") && strcmp(sh.type, "PSID")) {
    	// rprintf("Unknown SID format (%s)\n", sh.type);
	return (-1);
    }

    read_bigEndian_short(&sh.version, &buffer[0x4]);
    read_bigEndian_short(&sh.dataOffset, &buffer[0x6]);
    read_bigEndian_ushort(&sh.loadAddress, &buffer[0x8]);
    read_bigEndian_ushort(&sh.initAddress, &buffer[0xa]);
    read_bigEndian_ushort(&sh.playAddress, &buffer[0xc]);
    read_bigEndian_short(&sh.songs, &buffer[0xe]);
    read_bigEndian_short(&sh.startSong, &buffer[0x10]);

    read_bigEndian_int(&sh.speed, &buffer[0x12]);

    /*
    fread(&sh.name, sizeof(char[32]), 1, sidFile);
    fread(&sh.author, sizeof(char[32]), 1, sidFile);
    fread(&sh.released, sizeof(char[32]), 1, sidFile);
    */
	memcpy(&sh.name, &buffer[0x16], 32);
	memcpy(&sh.author, &buffer[0x36], 32);
	memcpy(&sh.released, &buffer[0x56], 32);
		    
    // sjekk om data ligger lagret i originalt format
    if (sh.loadAddress == 0x0) {
	read_littleEndian_ushort(&sh.loadAddress, &buffer[sh.dataOffset]);
	sh.dataOffset += 2;
	// platform_debug("\t (LoadAddress is: %x (first two bytes in dataOffset))\n", sh.loadAddress);
    }

    //rprintf("Format: %s\n", sh.type);
    //rprintf("Version: %d\n", (int) sh.version);
    //rprintf("DataOffset: %d\n", (int) sh.dataOffset);

    platform_debug("LoadAddress: %x\n", (int) sh.loadAddress);
    platform_debug("InitAddress: %x", sh.initAddress);

    if (sh.initAddress == 0x0) {
	platform_debug(" (Equals LoadAddress)");
	sh.initAddress = sh.loadAddress;
    }
    platform_debug("\n");

    //rprintf("PlayAddress: %d", sh.playAddress);

/*
    if (sh.playAddress == 0x0) {
	rprintf(" (Song installs an IRQ handler)");
    }
*/

    //rprintf("\n");
    //rprintf("Songs: %d (StartSong: %d)\n", sh.songs, sh.startSong);
    //rprintf("Speed: %d\n", sh.speed);

    if (sh.version == 2) {
		// s�rg for at vi ligger i rett posisjon
		/*
		    fseek(sidFile, 0x76, SEEK_SET);

		read_bigEndian_short(&sh.flags, sidFile);
		fread(&sh.startPage, sizeof(char), 1, sidFile);
		fread(&sh.pageLength, sizeof(char), 1, sidFile);
		*/
		
		read_bigEndian_short(&sh.flags, &buffer[0x76]);
		sh.startPage = buffer[0x78];
		sh.pageLength = buffer[0x79];
		sh.hz = 985248;		// vi defaulter til PAL hastigheten
	
		//rprintf("PSIDv2 Flags: %d\n", sh.flags);

		if (sh.flags != 0) {
			if (sh.flags & (1 << 0)) platform_debug("sidheader: Compute! MUS data\n");
			   else platform_debug("\tUses internal player (gooooood!)\n");
			if (sh.flags & (1 << 1) && !strcmp(sh.type, "RSID")) platform_debug("sidheader: RSID C64 Basic Flag\n");
			   else if (sh.flags & (1 << 1) && !strcmp(sh.type, "PSID")) platform_debug("sidheader: PlaySID Specific\n");
			   else platform_debug ("sidheader: C64 Compatible\n");
			if (sh.flags & (1 << 3)) { platform_debug("sidheader: SID is OK in NTSC mode\n"); sh.hz = 1022727; }
			if (sh.flags & (1 << 2)) { platform_debug("sidheader: SID is OK in PAL mode\n"); sh.hz = 985248; }
			if (sh.flags & (1 << 4)) platform_debug("sidheader: SID is OK on MOS6581\n");
			if (sh.flags & (1 << 5)) platform_debug("sidheader: SID is OK on MOS8580\n");
		}
		platform_debug("startPage: %d\n", sh.startPage);
		platform_debug("pageLength: %d\n", sh.pageLength);

    }
    
/*
    rprintf("Name: ");
    rprintfStr(&buffer[0x16]);
    rprintf("\nAuthor: ");
    rprintfStr(&buffer[0x36]);
    rprintf("\nCopyright: ");
    rprintfStr(&buffer[0x56]);
    rprintf("\n");

    rprintf("Size: %d\n", sh.size);
*/

    return (0);
}
