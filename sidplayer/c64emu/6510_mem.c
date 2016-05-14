#include "6510_mem.h"
#include "platform-support.h"

const static unsigned char kernal[] = {
	#include "kernal.901227-03.h"
};
const static unsigned char basic[] = {
	#include "basic.901226-01.h"
};

// Memory management
static void createMem(unsigned char page) {
    if (pages[page]) {
		return;
    }

    unsigned char *offset = (unsigned char*) malloc(sizeof(unsigned char[256]));
    if (offset == 0) {
    reveller->abort("ERROR: Memory Allocation failed!\n");
    }
    
    memset(offset, 0, 256);
    pages[page] = offset;
#ifdef DEBUG
    reveller->debug(" (Page %02x created)\n", page, offset);
#endif
}

// initier minnet
//  det betyr: sett opp stack page, og last inn programdata
void c64_initMem() {
    unsigned char buffer[256];
    unsigned char page;
    unsigned char offset;
    int i, pageStart;
    size_t readBytes;

    // free any allocated pages
    for (i = 0x0; i < 0x100; i++) {
		if (pages[i] != 0x0) {
#ifdef DEBUG
            reveller->debug("\tFreeing page %02x (pointing at %p)\n", i, pages[i]);
#endif
			free(pages[i]);
		}
    }
	
    memset(&pages, 0, sizeof(pages));
	
    createMem(0x00);	// ZeroPage
    createMem(0x01);	// Stack
    createMem(0xff);	// Her lagres blant annet NMI, RST og IRQ vektorer


    // legg inn SID fila i minne
    page = (sh.loadAddress >> 8);
    offset = sh.loadAddress;
    createMem(page);

    pageStart = sh.dataOffset;
    i = 0;

    readBytes = reveller->read(pageStart, 256 - offset, buffer);
    memcpy(pages[page] + offset, buffer, 256 - offset);
    
    page++;

	pageStart = sh.dataOffset + (256 - offset);
    readBytes = reveller->read(pageStart, 256, buffer);
    while (readBytes > 0) {
		createMem(page);
	
		memcpy(pages[page], buffer, readBytes);
		i++;
	
        readBytes = reveller->read(((i * 256) + pageStart), 256, buffer);
		page++;
    }
}

void c64_resetMem() {
    c64_storeMemRAMShort(0x000, 0x2f, 0x37);		// default ved powerON
    c64_storeMemRAMChar(0x00cc, 0x1);				// disable cursor blink
	
	// NTSC/PAL setting (=1 for PAL)
    c64_storeMemRAMChar(0x02a6, 0x1);
	// SETT 50 Hz timer
	if (sh.flags & (1 << 3)) {	// sjekk om headeren sier NTSC
        c64_storeMemRAMChar(0x02a6, 0x0);
		// SETT 60 Hz timer
	}
	
	if (!strcmp(sh.type, "RSID")) {
		// keylog
        c64_storeMemRAMShort(0x028f, 0x48, 0xeb);
		
		// software vector
        c64_storeMemRAMShort(0x0314, 0x31, 0xea);
	}
}

void c64_storeMemRAMChar(unsigned short addr, unsigned char data) {
    unsigned char page = addr >> 8;
    unsigned char offset = addr;

	if (! pages[page]) {
		createMem(page);
	}

	*(pages[page] + offset) = data;
}

// denne er ikke sikker i da den ikke sjekker page-crossing
void c64_storeMemRAMShort(unsigned short addr, unsigned char datal, unsigned char datah) {
    unsigned char page = addr >> 8;
    unsigned char offset = addr;

	if (! pages[page]) {
		createMem(page);
	}

	*(pages[page] + offset) = datal;
	offset++;
	*(pages[page] + offset) = datah;
}

void c64_storeMem(unsigned char s_data) {
    unsigned char page = effAddr >> 8;
    unsigned char offset = effAddr;
	
	data = (*(pages[0x0] + 0x1));
	// sjekk om IO er mappa inn
	if (data & 0x1 || data & 0x2) {
		switch (page) {
			case 0xd4:		// SID
			case 0xd5:
			case 0xd6:
			case 0xd7:		// jupp!  4 mirrors av SID
#ifdef DEBUG
                reveller->debug("\nSID Write: %04x, %02x", effAddr, s_data);
#endif
                c64_sid_register[offset & 0x1f] = s_data;
                reveller->sid_write(offset & 0x1f, s_data);
				break;
			case 0xd0:		// VIC-II
			case 0xd1:
			case 0xd2:
			case 0xd3:
#ifdef DEBUG
                reveller->debug("\nVIC Write: %04x, %02x (%04x)", effAddr, s_data, reg.pc);
#endif
                c64_vic_write(offset & 0x3f, s_data);
				break;
			case 0xdc:		// CIA 1
#ifdef DEBUG
                reveller->debug("\nCIA#1 Write: %04x, %02x (%04x)", effAddr, s_data, reg.pc);
#endif
                c64_cia_write(0, offset & 0xf, s_data);
				break;
			case 0xdd:		// CIA 2
#ifdef DEBUG
                reveller->debug("\nCIA#2 Write: %x, %x", offset, s_data);
#endif
                c64_cia_write(1, offset & 0xf, s_data);
				break;
			default:
				if (! pages[page]) {
					createMem(page);
				}
				
				*(pages[page] + offset) = s_data;
		}
	} else {
		if (! pages[page]) {
			createMem(page);
		}

		*(pages[page] + offset) = s_data;
	}
}

// fetches data for the current value in PC
void c64_fetchOP(void) {
    c64_loadMem(reg.pc);
}

// henter ut data fra ei minneadresse
static void loadMemRAM(unsigned char page, unsigned char offset) {
	if (! pages[page]) {
        reveller->debug("WARNING: at %x (op: %x): fetch from uninitialized PAGE (%x %x, returning 0x0)\n", reg.pc, data, page, offset);
		createMem(page);
		data = 0x0;
		return;
	}
	
	data = (*(pages[page] + offset));
}

void c64_loadMem(unsigned short addr) {
    unsigned char page = addr >> 8;
    unsigned char offset = addr;
    
	if (page < 0xa0) {
		loadMemRAM(page, offset);
		return;
	}
	
	// hent inn 0x0001
	data = (*(pages[0x0] + 0x1));

	switch (page >> 4) {
		case 0xa:
		case 0xb:
			// LORAM (BASIC ROM)
			if ((data & 0x3) == 0x3)		// iflg. programmeringsguiden swappes basic rom ut hvis kernal swappes ut
			{
				data = basic[((page - 0xa0) << 8 | offset)];
				return;
			} else {
				loadMemRAM(page, offset);
			}
			break;
		case 0xc:
			loadMemRAM(page, offset);
			break;
		case 0xd:
			// text IO space, then CHAR
			if (data & 0x1 || data & 0x2)
			{
				switch (page) {
					case 0xd0:
						data = 0;
						break;
					case 0xdc:
                        data = c64_cia_read(0, offset);
						break;
					case 0xdd:
                        data = c64_cia_read(1, offset);
						break;
					case 0xd4:
                        data = c64_sid_register[offset & 0x1f];
                        // reveller->debug("WARNING: Read from SID %x: %x\n", offset, data);
						// data = LES FRA SID (offset)
						break;
					default:
                        reveller->debug("WARNING: Unsupported LOAD from IO (%04x), returning RAM\n", addr);
						loadMemRAM(page, offset);
						break;
				}
			} else if (data & 0x4) {
                reveller->debug("WARNING: loadMem from CHAR, returning 0x0\n");
				data = 0;
			} else {
				loadMemRAM(page, offset);
			}
			break;
		case 0xe:
		case 0xf:
			if (data & 0x2) {
				data = kernal[((page - 0xe0) << 8 | offset)];
				return;
			} else {
				loadMemRAM(page, offset);
			}
			//sleep(3);
			break;
		default:
            reveller->debug("Yarr!  loadMemReal, her skal du bare ikke havne...\n");
	}
}

void c64_dumpMem() {
	int i, j;
	
	for (i = 0; i < 0x100; i++) {
        reveller->debug("PAGE %x\n", i);
		
		if (pages[i]) {
            reveller->debug("00: %x ", (*(pages[i])));
			for (j = 1; j < 0x100; j++) {
				if ((j % 0x10) == 0) {
                    reveller->debug("\n%x: ", j);
				}
                reveller->debug("%02x ", (*(pages[i] + j)));
			}
		}
        reveller->debug("\n");
	}
    reveller->debug("\n");

/*  - for KERNAL dump
	for (i = 0; i < 0x20; i++) {
        reveller->debug("KERNAL %x\n", (i + 0xe0 ));
		
        reveller->debug("00: %x ", kernal[i << 8]);
		for (j = 1; j < 0x100; j++) {
			if ((j % 0x10) == 0) {
                reveller->debug("\n%x: ", j);
			}
            reveller->debug("%x ", kernal[(i << 8) | j]);
		}
        reveller->debug("\n");
	}
*/
}

// memory fetch modes
void c64_memImm() {
	effAddr = ++reg.pc;
    c64_fetchOP();
#ifdef DEBUG
    reveller->debug(" %02x     ", data);
#endif
}

void c64_memAbsoluteAddr() {
    unsigned char page;
    unsigned char offset;

    // hent ut adresse
    ++reg.pc; c64_fetchOP();
    offset = data;
    ++reg.pc; c64_fetchOP();
    page = data;
    
    // adder ut riktig adresse
    effAddr = (page << 8) + offset;
    
#ifdef DEBUG
    reveller->debug(" %02x %02x  ", effAddr & 0xff, effAddr >> 8);
#endif
}

void c64_memAbsoluteAddrX() {
    unsigned char page;
    unsigned char offset;
    
    ++reg.pc; c64_fetchOP();
    offset = data;
    ++reg.pc; c64_fetchOP();
    page = data;
    
    effAddr = (page << 8) + offset + reg.x;
#ifdef DEBUG
    reveller->debug(" %02x %02x  ", offset, page);
#endif
}

void c64_memAbsoluteAddrY() {
    unsigned char page;
    unsigned char offset;
    
    ++reg.pc; c64_fetchOP();
    offset = data;
    ++reg.pc; c64_fetchOP();
    page = data;
    
    effAddr = (page << 8) + offset + reg.y;
#ifdef DEBUG
    reveller->debug(" %02x %02x  ", effAddr & 0xff, effAddr >> 8);
#endif
}

void c64_memZero() {
    unsigned char offset;
    
    ++reg.pc; c64_fetchOP();
    offset = data;
    
    effAddr = (0x00 << 8) + offset;
#ifdef DEBUG
    reveller->debug(" %02x     ", effAddr & 0xff);
#endif
}

void c64_memZeroX() {
    unsigned char offset;
    
    ++reg.pc; c64_fetchOP();
    offset = (data + reg.x) % 0x100;
    
    effAddr = (0x00 << 8) + offset;
#ifdef DEBUG
    reveller->debug(" %02x     ", effAddr & 0xff);
#endif
}

void c64_memZeroY() {
    unsigned char offset;
    
    ++reg.pc; c64_fetchOP();
    offset = (data + reg.y) % 0x100;
    
    effAddr = (0x00 << 8) + offset;
#ifdef DEBUG
    reveller->debug(" %02x     ", effAddr & 0xff);
#endif
}

void c64_memIndirectZeroX() {
    unsigned char page;
    unsigned char offset;
    unsigned char temp;

    // operand
    ++reg.pc; c64_fetchOP();
    temp = data + reg.x;

    // hent ut adresse fra ZP
    c64_loadMem(0x0 << 8 | (temp));
    offset = data;
    // her emulerer vi en faktisk BUG i 6510
    c64_loadMem(0x0 << 8 | ((temp + 1) % 0x100));
    page = data;
    
    // adder ut riktig adresse
    effAddr = (page << 8) + offset;
    
#ifdef DEBUG
    reveller->debug(" %02x     ", effAddr & 0xff);
#endif
}

void c64_memIndirectZeroY() {
    unsigned char page;
    unsigned char offset;
    unsigned char temp;

    // operand
    ++reg.pc; c64_fetchOP();
    temp = data;

#ifdef DEBUG
    reveller->debug(" %02x     ", data);
#endif

    // hent ut adresse fra ZP
    c64_loadMem(0x0 << 8 | (temp));
    offset = data;
    // her emulerer vi en faktisk BUG i 6510
    c64_loadMem(0x0 << 8 | ((temp + 1) % 0x100));
    page = data;
    
    // adder ut riktig adresse
    effAddr = (page << 8) + offset + reg.y;
}

void c64_memStack() {
    effAddr = (0x01 << 8) + reg.s;
}


