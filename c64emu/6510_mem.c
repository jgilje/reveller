#include "6510_mem.h"

// Minnehåndtering
void createMem(unsigned char page) {
    if (pages[page]) {
		return;
    }

    unsigned char *offset = (unsigned char*) malloc(sizeof(unsigned char[256]));
    if (offset == 0) {
		c64_debug("ERROR: Memory Allocation failed!\n");
    }
    
    memset(offset, 0, 256);
    pages[page] = offset;
#ifdef DEBUG
    c64_debug(" (Page %02x created)\n", page, offset);
#endif
}

// initier minnet
//  det betyr: sett opp stack page, og last inn programdata
void initMem() {
    unsigned char buffer[256];
    unsigned char page;
    unsigned char offset;
    int i, pageStart;
    size_t readBytes;

    // sjekk om vi allerede har gŒende et minne
    for (i = 0x0; i < 0x100; i++) {
		if (pages[i] != 0x0) {
#ifdef DEBUG
			c64_debug("\tFreeing page %02x (pointing at %p)\n", i, pages[i]);
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

    readBytes = c64_read_source(pageStart, 256 - offset, buffer);
    memcpy(pages[page] + offset, buffer, 256 - offset);
    
    page++;

	pageStart = sh.dataOffset + (256 - offset);
    readBytes = c64_read_source(pageStart, 256, buffer);
    while (readBytes > 0) {
		createMem(page);
	
		memcpy(pages[page], buffer, readBytes);
		i++;
	
		readBytes = c64_read_source(((i * 256) + pageStart), 256, buffer);
		page++;
    }
}

void resetMem() {
	storeMemRAMShort(0x000, 0x2f, 0x37);		// default ved powerON
	storeMemRAMChar(0x00cc, 0x1);				// disable cursor blink
	
	// NTSC/PAL setting (=1 for PAL)
	storeMemRAMChar(0x02a6, 0x1);
	// SETT 50 Hz timer
	if (sh.flags & (1 << 3)) {	// sjekk om headeren sier NTSC
		storeMemRAMChar(0x02a6, 0x0);
		// SETT 60 Hz timer
	}
	
	if (!strcmp(sh.type, "RSID")) {
		// keylog
		storeMemRAMShort(0x028f, 0x48, 0xeb);
		
		// software vector
		storeMemRAMShort(0x0314, 0x31, 0xea);
	}
}

void storeMemRAMChar(unsigned short addr, unsigned char data) {
    unsigned char page = addr >> 8;
    unsigned char offset = addr;

	if (! pages[page]) {
		createMem(page);
	}

	*(pages[page] + offset) = data;
}

// denne er ikke sikker i da den ikke sjekker page-crossing
void storeMemRAMShort(unsigned short addr, unsigned char datal, unsigned char datah) {
    unsigned char page = addr >> 8;
    unsigned char offset = addr;

	if (! pages[page]) {
		createMem(page);
	}

	*(pages[page] + offset) = datal;
	offset++;
	*(pages[page] + offset) = datah;
}

void storeMem(unsigned char s_data) {
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
				c64_debug("\nSID Write: %04x, %02x", effAddr, s_data);
				// if (sid_writes > 100) {dumpMem(); exit(0);}
#endif
				c64_sid_write(offset & 0x1f, s_data);
				break;
			case 0xd0:		// VIC-II
			case 0xd1:
			case 0xd2:
			case 0xd3:
#ifdef DEBUG
				c64_debug("\nVIC Write: %04x, %02x (%04x)", effAddr, s_data, reg.pc);
#endif
				vicWrite(offset & 0x3f, s_data);
				//exit(0);
				break;
			case 0xdc:		// CIA 1
#ifdef DEBUG
				c64_debug("\nCIA#1 Write: %04x, %02x (%04x)", effAddr, s_data, reg.pc);
#endif
				ciaWrite(0, offset & 0xf, s_data);
				break;
			case 0xdd:		// CIA 2
#ifdef DEBUG
				c64_debug("\nCIA#2 Write: %x, %x", offset, s_data);
#endif
				ciaWrite(1, offset & 0xf, s_data);
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

// denne er lik loadMem(), men er beregna på å hente fra PC
void fetchOP(void) {
    unsigned char page = reg.pc >> 8;
    unsigned char offset = reg.pc;
    
        // antagelse: henter ikke OPKODER fra BASIC ROM eller IO
	if (page < 0xe0) {
		data = (*(pages[page] + offset));
		return;
	}
	
	data = (*(pages[0x0] + 0x1));
	
        if (data & 0x2) {
                data = kernal[((page - 0xe0) << 8 | offset)];
                return;
        }

	loadMemRAM(page, offset);
	return;
}

// henter ut data fra ei minneadresse
void loadMemRAM(unsigned char page, unsigned char offset) {
	if (! pages[page]) {
		c64_debug("Advarsel ved %x (op: %x): henter inn fra PAGE som ikke er oppretta (%x %x, 0x0 returnert)\n", reg.pc, data, page, offset);
		createMem(page);
		data = 0x0;
		return;
	}
	
	data = (*(pages[page] + offset));
}

void loadMem(unsigned short addr) {
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
			if ((data & 0x1) && (data & 0x2))		// iflg. programmeringsguiden swappes basic rom ut hvis kernal swappes ut
			{
				//data = basic[((page - 0xa0) << 8 | offset)];
				c64_debug("Advarsel, loadMem fra BASIC ROM\n");
				data = 0x0;
				return;
			} else
				loadMemRAM(page, offset);
			break;
		case 0xc:
			loadMemRAM(page, offset);
			break;
		case 0xd:
			// test på IO space, deretter CHAR
			if (data & 0x1 || data & 0x2)
			{
				switch (page) {
					case 0xd0:
						data = 0;
						break;
					case 0xdc:
						data = ciaRead(0, offset);
						break;
					case 0xdd:
						data = ciaRead(1, offset);
						break;
					case 0xd4:
						// data = LES FRA SID (offset)
						break;
					default:
						c64_debug("ERROR: Unsupported LOAD from IO (%04x)\n", addr);
						loadMemRAM(page, offset);
						break;
				}
			} else if (data & 0x4) {
				c64_debug("ERROR: loadMem fra CHAR (og dette er ikke implementert enda!)\n");
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
			c64_debug("Yarr!  loadMemReal, her skal du bare ikke havne...\n");
	}
}

void dumpMem() {
	int i, j;
	
	for (i = 0; i < 0x100; i++) {
		c64_debug("PAGE %x\n", i);
		
		if (pages[i]) {
			c64_debug("00: %x ", (*(pages[i])));
			for (j = 1; j < 0x100; j++) {
				if ((j % 0x10) == 0) {
					c64_debug("\n%x: ", j);
				}
				c64_debug("%02x ", (*(pages[i] + j)));
			}
		}
		c64_debug("\n");
	}
	c64_debug("\n");

/*  - for KERNAL dump
	for (i = 0; i < 0x20; i++) {
		c64_debug("KERNAL %x\n", (i + 0xe0 ));
		
		c64_debug("00: %x ", kernal[i << 8]);
		for (j = 1; j < 0x100; j++) {
			if ((j % 0x10) == 0) {
				c64_debug("\n%x: ", j);
			}
			c64_debug("%x ", kernal[(i << 8) | j]);
		}
		c64_debug("\n");
	}
*/
}

// Her følger funksjoner for å hente ut data fra minnet, slik det skjer i 6510

void memImm() {
	effAddr = ++reg.pc;
	fetchOP();
#ifdef DEBUG
    c64_debug(" %02x     ", data);
#endif
}

void memAbsoluteAddr() {
    unsigned char page;
    unsigned char offset;

    // hent ut adresse
    ++reg.pc; fetchOP();
    offset = data;
    ++reg.pc; fetchOP();
    page = data;
    
    // adder ut riktig adresse
    effAddr = (page << 8) + offset;
    
#ifdef DEBUG
    c64_debug(" %02x %02x  ", effAddr & 0xff, effAddr >> 8);
#endif
}

void memAbsoluteAddrX() {
    unsigned char page;
    unsigned char offset;
    
    ++reg.pc; fetchOP();
    offset = data;
    ++reg.pc; fetchOP();
    page = data;
    
    effAddr = (page << 8) + offset + reg.x;
#ifdef DEBUG
    c64_debug(" %02x %02x  ", offset, page);
#endif
}

void memAbsoluteAddrY() {
    unsigned char page;
    unsigned char offset;
    
    ++reg.pc; fetchOP();
    offset = data;
    ++reg.pc; fetchOP();
    page = data;
    
    effAddr = (page << 8) + offset + reg.y;
#ifdef DEBUG
    c64_debug(" %02x %02x  ", effAddr & 0xff, effAddr >> 8);
#endif
}

void memZero() {
    unsigned char offset;
    
    ++reg.pc; fetchOP();
    offset = data;
    
    effAddr = (0x00 << 8) + offset;
#ifdef DEBUG
    c64_debug(" %02x     ", effAddr & 0xff);
#endif
}

void memZeroX() {
    unsigned char offset;
    
    ++reg.pc; fetchOP();
    offset = (data + reg.x) % 0x100;
    
    effAddr = (0x00 << 8) + offset;
#ifdef DEBUG
    c64_debug(" %02x     ", effAddr & 0xff);
#endif
}

void memZeroY() {
    unsigned char offset;
    
    ++reg.pc; fetchOP();
    offset = (data + reg.y) % 0x100;
    
    effAddr = (0x00 << 8) + offset;
#ifdef DEBUG
    c64_debug(" %02x     ", effAddr & 0xff);
#endif
}

void memIndirectZeroX() {
    unsigned char page;
    unsigned char offset;
    unsigned char temp;

    // operand
    ++reg.pc; fetchOP();
    temp = data + reg.x;

    // hent ut adresse fra ZP
    loadMem(0x0 << 8 | (temp));
    offset = data;
    // her emulerer vi en faktisk BUG i 6510
    loadMem(0x0 << 8 | ((temp + 1) % 0x100));
    page = data;
    
    // adder ut riktig adresse
    effAddr = (page << 8) + offset;
    
#ifdef DEBUG
    c64_debug(" %02x     ", effAddr & 0xff);
#endif
}

void memIndirectZeroY() {
    unsigned char page;
    unsigned char offset;
    unsigned char temp;

    // operand
    ++reg.pc; fetchOP();
    temp = data;

#ifdef DEBUG
    c64_debug(" %02x     ", data);
#endif

    // hent ut adresse fra ZP
    loadMem(0x0 << 8 | (temp));
    offset = data;
    // her emulerer vi en faktisk BUG i 6510
    loadMem(0x0 << 8 | ((temp + 1) % 0x100));
    page = data;
    
    // adder ut riktig adresse
    effAddr = (page << 8) + offset + reg.y;
}

void memStack() {
    effAddr = (0x01 << 8) + reg.s;
}


