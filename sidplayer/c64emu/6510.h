/*
    Emulering av MOS6510 (forhåpentligvis)

    
    (noe rotete, ikke alt stemmer.  se heller http://www.geocities.com/oneelkruns/asm1step.html)
    (jgilje tok kopi av denne sida 071204)
    - Adressering
	- implied (op. bruker ikke data)
	  TYA
	- immediate (angir en verdi direkte)
	  LDA #$99
	- absolute (angir en adresse hvor innholdet skal hentes fra)
	  LDA $3E32
	- zero page (angir en adresse i page $00, sparer altså 1 byte i koden)
	  LDA $23
	- indirect absolute (det angis en adresse, som igjen angir en adresse)
	  JMP ($2345)
	    1. henter ut offset fra $2345 (ex. $12) og page fra $2346 (ex. $34)
	    2. neste instruksjon starter i $3412
	- absolute indexed (adderer x/y register til _adressen_)
	  LDA $F453,X
	    1. $F453 + X (ex. $3) = $F453 + $3 = $F456
	    2. Accumulator = $F456
	- zero page indexed (adderer x/y register til en zeropage adresse)
	- indexed indirect (adderer x register til en en adresse (er denne kun zeropage?))
	  LDA ($B4,X)
	    1. B4 + X (ex. $6) = B4 + 6 = BA
	    2. $BA (ex. $12) og $BB (ex. $EE) leses ut
	    3. Accumulator = $EE12
	- indirect indexed (yarr!  altså motsatt av indexed indirect)
			   (henter ut indirect adresse, adderer så y til denne)
	  LDA ($B4),Y
	    1. henter ut $B4 (ex. $EE) og $B5 (ex. $12)
	    2. $12EE + Y (ex. $6) = $12EE + $6 = $12F4
	    3. Accumulator = $12F4
	- relativ (hopper frem eller tilbake, operand er en _signed_ byte)
		  (kan derfor hoppe i området -128 til +127)
    
    - Litt generelt dill
    
    - henter alltid inn to byte hver gang det leses fra minne (altså opcode og
      neste byte)
    - semi-pipelining: hvis forrige instruksjon ikke skrev til minne, så kan
      prosessoren hente inn neste opcode for neste instruksjon mens den utfører
      siste cpu-sykel.  Eks.: EOR #$FF:
        1. sykel: henter inn EOR ($49)
	2. sykel: henter inn parameter $FF
	3. sykel: utfør og lagre instruksjon til Acc., men samtidig hentes
	          opcode for neste instruksjon
 
    - NMI og IRQ bruker 7 sykler.  IRQ og BRK setter I flagget, NMI gjør ikke.
      Prosessoren gjør seg ferdig med gjeldende instruksjon før den går til
      Interrupt sekvensen.
*/

#ifndef _6510_H_
#define _6510_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// #include "rprintf.h"
// #include "sidfs.h"

#include "6510_mem.h"
#include "sidheader.h"
#include "cia.h"
#include "vic.h"

typedef struct Register {
    // hovedregister for aritmetiske operasjoner.  er forbundet direkte til
    // ALU.  derfor er mange opcodes kun tilgjengelige for accumulator
    unsigned char a;		// accumulator

    // index reg. x: hovedregister for � addressere data med "indices".
    // kan addressere "indexed indirect" - lar en ha en vektor tabell p�
    // "Zero Page"
    unsigned char x;		// register x

    // index reg. y: kun dette har en indirect address mode som gir tilgang
    // til hele minnet uten � bruke selv-modifiserende kode (skummelt...)
    unsigned char y;		// register y

    // Stack Pointer:
    //  256 bytes totalt stackminne (i omr�det 0x0100 - 0x01FF)
    //  S peker p� adresse i stackminnet, alts� (0x0100 + S)
    //  Kan leses og skrives vha. X register og opcodes TSX og TXS
    unsigned char s;		// stack pointer

    // Processor Status
    // kan leses via PUSH til Stack eller vha. interrupt
    // for � lese kun et flagg kan branch opcodes benyttes
    // for � sette flagg kan P hentes fra Stack, eller en kan
    //     benytte flag set/clear instruksjoner
    // Flaggene: (begynner med bit 8)
    //	8	N: Negative Flag (settes for enhver aritmetisk operasjon)
    // 		   Det ser ut til at dette flagget hentes fra bit 7 fra registeret
    //		   hvor en utf�rte aritmetisk operasjon (a, x el. y)
    //	7	V: oVerflow Flag (settes ved addisjon og subtraksjon, PLP, CLV og BIT
    // 		   instruksjonene, samt hardware signal S0)
    //	6	1: Ubrukt ("to current knowledge, this flag is always 1")
    //	5	B: Break Flag (for � skille mellom soft. og hard. interrupt)
    //		   Denne er alltid 1, bortsett fra n�r P register blir PUSH -> S n�r
    //		   en f�r Jump til en hardware interrupt rutine. (Les ogs� om NMI og BRK)
    //	4	D: Decimal mode Flag (angir Decimal modus, denne er oftest 0)
    //		   Les spesielt p� ADC, SBC og ARR opcodes
    //	3	I: Interrupt disable Flag (hindrer CPU i � g� til IRQ handle vector (0xFFFE)
    //		   hvis en IRQ forekommer.  Dette flagg settes automatisk etter mottak av en IRQ)
    //	2	Z: Zero Flag (gjelder for instuksjoner som N).  Settes hvis et aritmetisk register
    //		   blir satt til 0.  Opererer annerledes ved desimaler
    //	1	C: Carry Flag (opptrer som 9. bit, ved overflow er dette flagg 0.  Sammenligninger
    //		   forventer C=1, og D=0)

    unsigned char p;

// TODO: finn ut hva som er hensiktsmessig, low og high, eller kombinert
    // Program Counter kan leses ved PUSH til Stack, enten via
    //  subroutine eller interrupt

    // program counter
    unsigned short pc;		// program counter
//    unsigned char page;		// program counter, low
//    unsigned char offset;	// program counter, high
} Register;

// Registeret
Register reg;

// innholdet er den byte som er gjeldende i �yeblikket
unsigned char data;
//volatile int data;
// den effektive adresse
unsigned short effAddr;

// Tabellen over opcodes
void (*opcodes[256])(void);

// dette flagget bryter interpret funksjonen, typisk ved RTS i bin�rkoden
// (eks.: stack overflow)
unsigned char work;
unsigned char SIDDriverPage;
unsigned char c64_current_song;

// signatur for en uimplementert opkode
void Un_imp(void);

void IRQTrigger(void);
void NMITrigger(void);
void initSong();
void setSubSong(unsigned char);
void PrintOpcodeStats(void);
int32_t c64_play(void);
void interpret(int i, unsigned short addr);

// Definer reg.p sine flag, (giljen bare roter n�r han skal trikse med dem)
#define FLAG_N 0x80
#define FLAG_V 0x40
#define FLAG_U 0x20	// Unused
#define FLAG_B 0x10
#define FLAG_D 0x08
#define FLAG_I 0x04
#define FLAG_Z 0x02
#define FLAG_C 0x01


#endif
