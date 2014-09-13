/* SID filer kommer i 4 varianter:
    PSID v1
    PSID v2 (utvikling av v1)
    PSID v2NG (utvikling av v2)
    RSID (spesialtilfelle av v2NG)
*/

#ifndef _SIDHEADER_H_
#define _SIDHEADER_H_

#include <stdio.h>
// #include "rprintf.h"
// #include "93cXX.h"

typedef struct sidHeader {
    // 0x00
    char type[5];	// RSID / PSID
    // 0x04
    short version;	// RSID og PSIDv2NG == 0002
    // 0x06
    short dataOffset;	// 0x0076 for v1, 0x007C for v2
			// Offsett i _SIDfila_
    // 0x08
    unsigned short loadAddress;	// C64 minnelokasjon for C64 data (little endian)
			// (som betyr lowByte, highByte)
			// (== 0: de to første byte i dataseksjon
			//        inneholder startposisjon (og da i
			//        lav - høy format, dette gjelder for
			//	  alle RSID)
    // 0x0A
    unsigned short initAddress;	// Addressen som initialiserer sangen, dette er
			// en subroutine som tar imot et sangnummer fra
			// 6510 Accumulator
			// (== 0: initAddress == loadAddress)
    // 0x0C
    unsigned short playAddress;	// Adresse til en subroutine som kalles for å
			// gjengi en sammenhengende lyd
			// (== 0: initAddress forventes å legge inn en
			//	  IRQ handler, som så kaller playeren,
			//	  dette gjelder for alle RSID)
    // 0x0E
    short songs;	// antall sanger som _kan_ bli initialisert av
			// initAddress.  min.: 1, maks.: 256
    // 0x10
    short startSong;	// første sang som spilles i fila, (==0: 1)
    // 0x12
    int speed;		// 32 bit, bigendian.  Hvert bit angir hastigheten
			// til de 32 første sangene.  Over 32 så settes hastighet
			// til den gjeldende for nr. 32.  Bit 0 == hastighet for
			// sang 1.  Bit'et betyr: 0: vertical blank interrupt
			// som igjen betyr 50Hz PAL, 60Hz NTSC.  1: CIA1 timer
			// interrupt (default 60Hz).  (for RSID er hele speed == 0)
    // De neste 3 strengene lagres slik at vi ikke trenger den ene byte, slik som i
    // type, da sidformatet ordner dette for oss.
    // 0x16
    char name[32];
    // 0x36
    char author[32];
    // 0x56
    char released[32];
    
    // 0x76
    // Nå må det sjekkes i programkoden: for SIDv1 begynner dataseksjonen på 0x76, for
    // SIDv2* så fortsetter headerinfoen
    short flags;	// 16 bit, bigendian.
			// 0: formatet til binærdata
			//    0 - innebygd musikkspiller, 1 - Compute! music player må 'merges'
			// 1: PlaySID spesifikk SID (PlaySID samples)
			//    0 - C64 Kompatibel, 1 PlaySID spesifikk (v2NG), C64 BASIC flag (RSID)
			// 2 og 3: utgjør 4 bit om hastigheten (v2NG)
			//    0 - Ukjent, 1 - PAL, 2 - NTSC, 3 - Begge
			// 4 og 5: angir SID modell (v2NG)
			//    0 - Ukjent, 1 - 6581, 2 - 8580, 3 - Begge
			// 6 til 15: Ubrukt
    // 0x78
    char startPage;	// Angir hvor en finner største ledige minneblokk (page),
			// innenfor "driver området"(?) (v2NG)
			// (== 00: SID fila skriver ikke utenfor dataområdet 
			//         innenfor "driver området")
			// (== FF: ingen ledige minneblokker)
    // 0x79
    char pageLength;	// Antall ledige minneblokker (pages) etter startPage (v2NG)
			// Denne er alltid 00, når startPage er 00 eller FF
    
    // 0x7C
    // Dataområdet til SIDv2 og etterkommere begynner her
    // Husk å sjekke loadAddress, de to første BYTE her _kan_ være loadAddress

	// angir PAL/NTSC kompabilitet
	int hz;
} sidHeader;

sidHeader sh;
int parseHeader(void);

#endif
