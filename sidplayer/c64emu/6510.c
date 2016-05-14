#include "6510.h"
#include <stdint.h>

unsigned char instruction[3];	// vi leser maksimalt inn 3 byte
FILE* file;

// the emulator once ran on a chip with a total of 64kb.
// from that design, we allocated C64-pages only as needed
// this system was good enough to run up to ~45kb songs
unsigned char *pages[256];
unsigned char work = 0;

static void setPC(short pc) {
    // reg.offset = pc;
    // reg.page = ((pc & 0xFF00) >> 8);
    reg.pc = pc;
}

static void evalNZ(unsigned char byte) {
    reg.p &= 0x7d;
    if (byte & FLAG_N) reg.p |= FLAG_N;	// n
    if (byte == 0) reg.p |= FLAG_Z;	// z
    // se kommentaren i header, for fyldig info
}

// the actual 6510-instructions
#include "6510_alu.c"
#include "6510_move.c"
#include "6510_branch.c"

static void (*opcodes[256])(void);
static void Un_imp(void);

void c64_printOpcodeStats(void) {
    int unimplemented = 0;
    int implemented = 0;
    int x;

    for (x = 0; x < 256; x++) {
        if (opcodes[x] == &Un_imp) unimplemented++;
        else implemented++;
    }

    reveller->debug("Implemented OpCodes: %d of 256 (unimplemented %d)\n", implemented, unimplemented);
}

static void Un_imp(void) {
    c64_printOpcodeStats();
    c64_dumpMem();

    reveller->abort("\nCall to unimplemented function %x at %d\n", data, reg.pc);
}

static void (*opcodes[])(void) = 	// alle opcodes for prosessoren
{

// 0x0,  0x1,     0x2,     0x3,     0x4,     0x5,     0x6,     0x7
// 0x8,  0x9,     0xA,     0xB,     0xC,     0xD,     0xE,     0xF
// 0x00
&BRK_, &ORA_izx, &Un_imp, &Un_imp, &Un_imp, &ORA_zp, &ASL_zp, &Un_imp, 
&PHP_, &ORA_imm, &ASL_imp, &Un_imp, &Un_imp, &ORA_abs, &ASL_abs, &Un_imp, 
// 0x10
&BPL_, &ORA_izy, &Un_imp, &Un_imp, &Un_imp, &ORA_zpx, &ASL_zpx, &Un_imp, 
&CLC_, &ORA_absy, &Un_imp, &Un_imp, &Un_imp, &ORA_absx, &ASL_absx, &Un_imp, 
// 0x20
&JSR_, &AND_izx, &Un_imp, &Un_imp, &BIT_zp, &AND_zp, &ROL_zp, &Un_imp, 
&PLP_, &AND_imm, &ROL_imp, &Un_imp, &BIT_abs, &AND_abs, &ROL_abs, &Un_imp, 
// 0x30
&BMI_, &AND_izy, &Un_imp, &Un_imp, &Un_imp, &AND_zpx, &ROL_zpx, &Un_imp, 
&SEC_, &AND_absy, &Un_imp, &Un_imp, &Un_imp, &AND_absx, &ROL_absx, &Un_imp, 
// 0x40
&RTI_, &EOR_izx, &Un_imp, &Un_imp, &Un_imp, &EOR_zp, &LSR_zp, &Un_imp, 
&PHA_, &EOR_imm, &LSR_imp, &Un_imp, &JMP_abs, &EOR_abs, &LSR_abs, &Un_imp, 
// 0x50
&BVC_, &EOR_izy, &Un_imp, &Un_imp, &Un_imp, &EOR_zpx, &LSR_zpx, &Un_imp, 
&CLI_, &EOR_absy, &Un_imp, &Un_imp, &Un_imp, &EOR_absx, &LSR_absx, &Un_imp, 
// 0x60
&RTS_, &ADC_izx, &Un_imp, &Un_imp, &Un_imp, &ADC_zp, &ROR_zp, &Un_imp, 
&PLA_, &ADC_imm, &ROR_imp, &Un_imp, &JMP_ind, &ADC_abs, &ROR_abs, &Un_imp, 
// 0x70
&BVS_, &ADC_izy, &Un_imp, &Un_imp, &Un_imp, &ADC_zpx, &ROR_zpx, &Un_imp, 
&SEI_, &ADC_absy, &Un_imp, &Un_imp, &Un_imp, &ADC_absx, &ROR_absx, &Un_imp, 

// 0x0,  0x1,     0x2,     0x3,     0x4,     0x5,     0x6,     0x7
// 0x8,  0x9,     0xA,     0xB,     0xC,     0xD,     0xE,     0xF
// 0x80
&Un_imp, &STA_izx, &Un_imp, &Un_imp, &STY_zp, &STA_zp, &STX_zp, &Un_imp, 
&DEY_, &Un_imp, &TXA_, &Un_imp, &STY_abs, &STA_abs, &STX_abs, &Un_imp, 
// 0x90
&BCC_, &STA_izy, &Un_imp, &Un_imp, &STY_zpx, &STA_zpx, &STX_zpy, &Un_imp, 
&TYA_, &STA_absy, &TXS_, &Un_imp, &Un_imp, &STA_absx, &Un_imp, &Un_imp, 
// 0xA0
&LDY_imm, &LDA_izx, &LDX_imm, &Un_imp, &LDY_zp, &LDA_zp, &LDX_zp, &Un_imp, 
&TAY_, &LDA_imm, &TAX_, &Un_imp, &LDY_abs, &LDA_abs, &LDX_abs, &Un_imp, 
// 0xB0
&BCS_, &LDA_izy, &Un_imp, &Un_imp, &LDY_zpx, &LDA_zpx, &LDX_zpy, &Un_imp, 
&CLV_, &LDA_absy, &TSX_, &Un_imp, &LDY_absx, &LDA_absx, &LDX_absy, &Un_imp, 
// 0xC0
&CPY_imm, &CMP_izx, &Un_imp, &Un_imp, &CPY_zp, &CMP_zp, &DEC_zp, &Un_imp, 
&INY_, &CMP_imm, &DEX_, &Un_imp, &CPY_abs, &CMP_abs, &DEC_abs, &Un_imp, 
// 0xD0
&BNE_, &CMP_izy, &Un_imp, &Un_imp, &Un_imp, &CMP_zpx, &DEC_zpx, &Un_imp, 
&CLD_, &CMP_absy, &Un_imp, &Un_imp, &Un_imp, &CMP_absx, &DEC_absx, &Un_imp, 
// 0xE0
&CPX_imm, &SBC_izx, &Un_imp, &Un_imp, &CPX_zp, &SBC_zp, &INC_zp, &Un_imp, 
&INX_, &SBC_imm, &NOP_, &Un_imp, &CPX_abs, &SBC_abs, &INC_abs, &Un_imp, 
// 0xF0
&BEQ_, &SBC_izy, &Un_imp, &Un_imp, &Un_imp, &SBC_zpx, &INC_zpx, &Un_imp, 
&SED_, &SBC_absy, &Un_imp, &Un_imp, &Un_imp, &SBC_absx, &INC_absx, &Un_imp, 

};

uint32_t interpretMain(void) {
	uint32_t cycles = 0;
	while (work) {
        c64_fetchOP();
#ifdef DEBUG
        reveller->debug(" PC   A  X  Y  SP  DR PR NV-BDIZC Instr.\n", data, reg.pc);
        reveller->debug("%04x %02x %02x %02x 01%x ", reg.pc, reg.a, reg.x, reg.y, reg.s);
        reveller->debug("%x %x ", *(pages[0]), *(pages[0] + 1));
		
		int i;
		for (i = 7; i >= 0; i--) {
            if (reg.p & (1 << i)) reveller->debug("1"); else reveller->debug("0");
		}
		
        reveller->debug(" %02x", data);
#endif

		opcodes[data]();
		reg.pc++;
		cycles += 3;

#ifdef DEBUG
        reveller->debug("\n\n");
#endif
	}
	return cycles;
}

static uint32_t c64_trigger_irq(void);
static uint32_t c64_trigger_nmi(void);
void interpretInit(unsigned char song) {
	*(pages[0x0] + 0x1) = getIOPort(sh.initAddress);
	c64_current_song = (song == 0) ? (sh.startSong - 1) : (song - 1);
	reg.a = c64_current_song;
    reveller->debug("c64_setSubSong(): running initAddress\n");
	reg.p |= FLAG_I;

	setPC(sh.initAddress);
	uint32_t cycles = 0;
	work = 1;
	while (work) {
        c64_fetchOP();
		opcodes[data]();
		reg.pc++;

		/* Until we correct the opcode handling, assume each opcode is 3 cycles */
		c64_cia_update_timers(3);
		c64_vic_update_timer(3);
		cycles += 3;

		if (c64_cia_nmi()) {
            // reveller->debug("interpretMain(): NMI\n");
			cycles += c64_trigger_nmi();
			reg.pc -= 2;
			work = 1;
		}

		if ((c64_cia_irq() || c64_vic_irq()) && !(reg.p & FLAG_I)) {
            // reveller->debug("interpretMain(): IRQ\n");
			cycles += c64_trigger_irq();
			reg.pc -= 2;
			work = 1;
		}
	}

	reg.p &= ~FLAG_I;
}

// interpret i times playAddr from address addr
void interpret(int i, unsigned short addr) {
	while (i > 0) {
		work = 1;
		setPC(addr);
		interpretMain();
		i--;
	}
}

void triggerInterrupt(void) {
	unsigned char stack_start = reg.s;
    c64_memStack();
    c64_storeMem(reg.pc >> 8);
	reg.s--;
    c64_memStack();
    c64_storeMem(reg.pc & 0xff);
	reg.s--;
    c64_memStack();
    c64_storeMem(reg.p);
	reg.s--;
	if (reg.s > stack_start) {
        reveller->abort("Stack Overflow");
	}
	
	work = 1;
}

uint32_t c64_trigger_irq(void) {
	unsigned short addr = (*(pages[0xff] + 0xff) << 8) | *(pages[0xff] + 0xfe);
	triggerInterrupt();
	reg.p |= FLAG_I;
	setPC(addr);
#ifdef DEBUG
    reveller->debug("\n****************************************************\n");
    reveller->debug("IRQ Vector@%04x\n", addr);
    reveller->debug("****************************************************\n");
#endif

	return interpretMain();
}

uint32_t c64_trigger_nmi() {
	unsigned short addr = (*(pages[0xff] + 0xfb) << 8) | *(pages[0xff] + 0xfa);
	triggerInterrupt();
	setPC(addr);

#ifdef DEBUG
    reveller->debug("\n****************************************************\n");
    reveller->debug("NMI Vector@%04x\n", addr);
    reveller->debug("****************************************************\n");
#endif

	return interpretMain();
}

int32_t c64_next_trigger(void) {
	int32_t cia_next = c64_cia_next_timer();
	int32_t vic_next = c64_vic_next_timer();

	if (cia_next > 0 && vic_next > 0) {
		if (cia_next < vic_next) {
			return cia_next;
		} else {
			return vic_next;
		}
	}
	
	if (cia_next > 0) {
		return cia_next;
	}
	if (vic_next > 0) {
		return vic_next;
	}
	
    reveller->debug("c64_next_trigger(): no IRQ?\n");
	return 20000;
}

int32_t c64_play(void) {
	uint32_t sleep_time = 0;
	uint32_t next = 0;
	int interrupted = 0;
	
	while (! interrupted) {
		next = c64_next_trigger();
		c64_cia_update_timers(next);
		c64_vic_update_timer(next);
		
		if (c64_cia_nmi()) {
			interrupted = 1;
			c64_trigger_nmi();
		}
		if (c64_cia_irq() || c64_vic_irq()) {
			interrupted = 1;
			c64_trigger_irq();
		}
		
		sleep_time += next;
	}
	
	return sleep_time;
}

unsigned char getIOPort(unsigned short address) {
	if (sh.version == 1) {
		return 0x34;
	} else {
		if (address < 0xa000) {
			return 0x37;
		} else if (address < 0xd000) {
			return 0x36;
		} else if (address >= 0xe000) {
			return 0x35;
		}
	}
	
	return 0x0;		// kompilatoren klager
}

void installSIDDriver(void) {
	int pageCounter = 0x04;
	int freePage = 0;
	unsigned char IOPort = 0x37;
	unsigned short address = 0;

	if (sh.version >= 2 && sh.startPage > 0 && sh.pageLength > 0) {
		freePage = sh.startPage;
	} else {
		for (pageCounter = 0x04; pageCounter < 0xa0; pageCounter++) {
			if (! pages[pageCounter]) {
				freePage = pageCounter;
				break;
			}
		}
	}
	
	if (freePage == 0) {
        reveller->abort("Failed to retrieve a free page for SID Driver\n");
	}
	IOPort = getIOPort(sh.playAddress);
    reveller->debug("installSIDDriver(): installing at freePage: %x, IOPort: %x\n", freePage, IOPort);

#ifdef DEBUG
    reveller->debug("Installing SID Driver in free page %x\n", freePage);
#endif
	address = freePage << 8;
	
	// retrieve correct IOPort before running SID
    c64_storeMemRAMShort(address, 0xa9, IOPort); address += 2;
    c64_storeMemRAMShort(address, 0x85, 0x1); address += 2;
	// JSR to PlayAddress
    c64_storeMemRAMChar(address, 0x20); address += 1;
    c64_storeMemRAMShort(address, (sh.playAddress & 0xff), (sh.playAddress >> 8)); address += 2;
	// reset IOPort to 0x37
    c64_storeMemRAMShort(address, 0xa9, 0x37); address += 2;
    c64_storeMemRAMShort(address, 0x85, 0x1); address += 2;
	// JMP to yourself, the emulator will figure out what's happening
	/*
	storeMemRAMChar(address, 0x4c); address += 1;
	storeMemRAMShort(address, 0xb, freePage);
	*/
	// RTI
    c64_storeMemRAMChar(address, 0x40);

	SIDDriverPage = (freePage & 0xff);
}

static void initCPU(void) {
	memset(&reg, 0, sizeof(reg));
	reg.p = FLAG_U;		// NOT USED
	reg.s = 0xff;
}

static void initSong() {
	// initier minne og registre
	parseHeader();
	initCPU();
    c64_initMem();
    c64_resetMem();
}

void c64_sid_init(void) {
	int i;
	for (i = 0; i <= 28; i++) {
        reveller->sid_write(i, 0);
	}
}

void c64_setSubSong(unsigned char song) {
	c64_cia_init();
	c64_vic_init();
	c64_sid_init();
	initSong();

    c64_storeMemRAMShort(0xfffa, 0x43, 0xfe);
    c64_storeMemRAMShort(0xfffe, 0x48, 0xff);

	if (! strcmp(sh.type, "RSID")) {
        reveller->debug("c64_setSubSong(): running KERNAL init\n");
		interpret(1, 0xff84);
	}
	// TODO remove if not needed
	// storeMemRAMShort(0x0314, 0x0, SIDDriverPage);

	// run INIT adress with songNo in reg.a
	interpretInit(song);
	
	if (! strcmp(sh.type, "PSID")) {
		installSIDDriver();

		if (sh.playAddress) {
            c64_storeMemRAMShort(0xfffe, 0x0, SIDDriverPage);
		}
		
		// sett opp CIA#1
		if (sh.speed & (1 << c64_current_song)) {
            reveller->debug("CIA#1 timer enabled from sh.speed\n");
            c64_cia_write(0, 0xd, 0x81);
            c64_cia_write(0, 0xe, 0x1);	// enable CIA#1TimerA
		} else {
            reveller->debug("c64_setSubSong(): VIC Timer\n");
            c64_vic_write(0x1a, 0x1);	// enable VIC Interrupts
		}
	}
}

void c64_set_platform(struct reveller_platform *platform) {
    reveller = platform;
}
