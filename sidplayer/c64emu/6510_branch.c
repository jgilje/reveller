// Flag/Branching

// Break
static void BRK_(void) {		// 0x00
	unsigned char page;
	unsigned char offset;
	
	if (reg.s == 0x00) {
#ifdef DEBUG
//		platform_debug("Stack Overflow!\n");
//		platform_debug("Emulation Stopped\n");
#endif
		work = 0;
		return;
    }

	
	// feng tak i neste adresse, BRK har 1 byte padding
	reg.pc++;

	memStack();
	storeMem(reg.pc >> 8);
	reg.s--;
	memStack();
	storeMem(reg.pc & 0xff);
	reg.s--;

	memStack();
	storeMem(reg.p | FLAG_U | FLAG_B);
	reg.s--;
	
	reg.p |= FLAG_I;
	
	loadMem(0xfffe);
	offset = data;
	loadMem(0xffff);
	page = data;
	
#ifdef DEBUG	
	platform_debug("        BRK");
#endif	

	reg.pc = ((page << 8) | offset);
	reg.pc--;

	platform_debug("BRK: Denne bør sjekkes litt\n");
	platform_debug("BRK: Jump to: %x", reg.pc);
	//sleep(3);
	//exit(0);	
}

// NOP (denne hører egentlig ikke hjemme noe sted)
static void NOP_(void) {	// 0xea
#ifdef DEBUG	
	platform_debug("        NOP");
#endif	
}

// Flag
static void BIT_(void) {	// 0x2C
    // BIT sets the Z flag as though the value in the address tested were ANDed
    // with the accumulator. The N and V flags are set to match bits 8 and 7
    // respectively in the value stored at the tested address.

    loadMem(effAddr);
    // Denne setter flaggene N og V, samt & mot A i Z
    // bit: 8, 7 og 2
    //data = 0xe0;

    reg.p &= 0x3d;
    
    reg.p |= (data & FLAG_N);
    reg.p |= (data & FLAG_V);
    reg.p |= (((data & reg.a) == 0x00) << 1);

#ifdef DEBUG
    platform_debug("BIT");
#endif
}

static void BIT_abs(void) {	// 0x2C
	memAbsoluteAddr();
	BIT_();
}

static void BIT_zp(void) {		// 0x24
	memZero();
	BIT_();
}

static void CLC_(void) {		// 0x18
    // Clear Carry
    reg.p &= 0xfe;
#ifdef DEBUG
    platform_debug("        CLC");
#endif
}

static void SEC_(void) {		// 0x38
    // Set Carry
    reg.p |= FLAG_C;
#ifdef DEBUG
    platform_debug("        SEC");
#endif
}

static void CLI_(void) {		// 0x58
    // Clear Interrupt
    reg.p &= 0xfb;
#ifdef DEBUG
    platform_debug("        CLI");
#endif
}

static void SEI_(void) {		// 0x78
    // Set Interrupt
    reg.p |= FLAG_I;
#ifdef DEBUG
    platform_debug("        SEI");
#endif
}

static void CLV_(void) {		// 0xB8
    // Clear Overflow
    reg.p &= 0xbf;
#ifdef DEBUG
    platform_debug("        CLV");
#endif
}

static void CLD_(void) {		// 0xD8
    // Clear Decimal
    reg.p &= 0xf7;
#ifdef DEBUG
    platform_debug("        CLD");
#endif
}

static void SED_(void) {		// 0xF8
    // Clear Decimal
    reg.p |= FLAG_D;
#ifdef DEBUG
    platform_debug("        SED");
#endif
}

// Branching
static void Branch_(void) {	// Wrapper for flere branch funksjoner
    memImm();
    // loadMem(effAddr);
#ifdef DEBUG
//    platform_debug("\tBranch: %02x (%d, sbyte: %02x (%d))\n", data, data, (signed char)data, (signed char)data);
#endif
	reg.pc += (signed char) data;
	reg.pc--;
}

static void BPL_(void) {		// 0x10
    // Branch on N=0
#ifdef DEBUG
    platform_debug("        BPL");
#endif
    if (! (reg.p & FLAG_N)) Branch_();
	reg.pc++;
}

static void BMI_(void) {		// 0x30
    // Branch on N=1
#ifdef DEBUG
    platform_debug("        BMI");
#endif
    if (reg.p & FLAG_N) Branch_();
	reg.pc++;
}

static void BVC_(void) {		// 0x50
    // Branch on V=0	// FAAAAAAAAN!  Den kosta giljen en halv dag med arbeid
#ifdef DEBUG
    platform_debug("        BVC");
#endif
    if (! (reg.p & FLAG_V)) Branch_();
	reg.pc++;
}

static void BVS_(void) {		// 0x70
    // branch on V=1
#ifdef DEBUG
    platform_debug("        BVS");
#endif
    if (reg.p & FLAG_V) Branch_();
	reg.pc++;
}

static void BCC_(void) {		// 0x90
    // Branch on C=0
#ifdef DEBUG
    platform_debug("        BCC");
#endif
    if (! (reg.p & FLAG_C)) Branch_();
	reg.pc++;
}

static void BCS_(void) {		// 0xB0
    // Branch on C=1
#ifdef DEBUG
    platform_debug("        BCS");
#endif
    if (reg.p & FLAG_C) Branch_();
	reg.pc++;
}

static void BNE_(void) {		// 0xD0
    // Branch on Z=0
#ifdef DEBUG
    platform_debug("        BNE");
#endif
    if (! (reg.p & FLAG_Z)) Branch_();
	reg.pc++;
}

static void BEQ_(void) {		// 0xF0
    // Branch on Z=1
#ifdef DEBUG
    platform_debug("        BEQ");
#endif
    if (reg.p & FLAG_Z) Branch_();
	reg.pc++;
}

static void JMP_abs(void) {	// 0x4c
    memAbsoluteAddr();
		
	// check if we entered an endless loop
	if (effAddr == (reg.pc - 2)) {
		work = 0;
		// platform_debug("Infinite Loop\nEmulation Stopped\n");
		return;
	}

    reg.pc = effAddr;
	reg.pc--;

#ifdef DEBUG
    platform_debug("JMP");
#endif
}

static void JMP_ind(void) {	// 0x6c
	unsigned char page;
	unsigned char offset;
	unsigned char jmp_page;
	unsigned char jmp_offset;
	
    memAbsoluteAddr();
    page = effAddr >> 8;
	offset = effAddr & 0xff;
	
	loadMem(effAddr);
	jmp_offset = data;
	offset++;
	loadMem((page << 8) | offset);
	jmp_page = data;
	
	reg.pc = (jmp_page << 8) | jmp_offset;
	reg.pc--;

#ifdef DEBUG
    platform_debug("JMP");
#endif
}


// Subroutines
static void JSR_(void) {		// 0x20
	// feng tak i neste adresse
	unsigned short jmpAddr;
	memAbsoluteAddr();
	jmpAddr = effAddr;
	
	// lagre adresse til (nextop - 1) i stack 
	
	memStack();
	storeMem(reg.pc >> 8);
	reg.s--;
	memStack();
	storeMem(reg.pc & 0xff);
	reg.s--;

#ifdef DEBUG	
	platform_debug("JSR");
#endif	

	reg.pc = jmpAddr;
	reg.pc--;
}

static void RTI_(void) {		// 0x40
    unsigned char page;
	unsigned char offset;
	
	if (reg.s == 0xff) {
#ifdef DEBUG
//		platform_debug("Stack Overflow!\n");
//		platform_debug("Emulation Stopped\n");
#endif
		work = 0;
		return;
    }
	
	reg.s++;
	memStack();
	loadMem(effAddr);
	reg.p = data;
	
	reg.s++;
	memStack();
	loadMem(effAddr);
	offset = data;
	reg.s++;
	memStack();
	loadMem(effAddr);
	page = data;
	
    reg.pc = ((page << 8) | offset);    
    reg.pc++;
	
#ifdef DEBUG	
	platform_debug("        RTI");
#endif

	// sjekk om vi er ferdige med et IRQ kall
	if (reg.s == 0xff) {
		work = 0;
	}
	
	return;
}

static void RTS_(void) {		// 0x60
    unsigned char page;
	unsigned char offset;
	
	if (reg.s == 0xff) {
#ifdef DEBUG
//		platform_debug("Stack Overflow!\n");
//		platform_debug("Emulation Stopped\n");
#endif
		work = 0;
		return;
    }
	
	
	// denne er ikke helt bra...
	// vi burde lagt opp til å kjøre en driver istedet, ala sidplay2
//	if ((reg.p & FLAG_I) && (reg.s == 0xf9)) {
//		reg.s = 0xfc;
//		RTI_();
//		return;
//	}
		
	reg.s++;
	memStack();
	loadMem(effAddr);
	offset = data;
	reg.s++;
	memStack();
	loadMem(effAddr);
	page = data;
	
    reg.pc = ((page << 8) | offset);    
    // reg.pc++;
	
#ifdef DEBUG	
	platform_debug("        RTS");
#endif
}

