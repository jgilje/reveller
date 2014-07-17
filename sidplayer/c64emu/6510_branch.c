// Flag/Branching

// Break
void BRK_(void) {		// 0x00
	unsigned char page;
	unsigned char offset;
	
	if (reg.s == 0x00) {
#ifdef DEBUG
//		c64_debug("Stack Overflow!\n");
//		c64_debug("Emulation Stopped\n");
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
	c64_debug("        BRK");
#endif	

	reg.pc = ((page << 8) | offset);
	reg.pc--;

	c64_debug("BRK: Denne bør sjekkes litt\n");
	c64_debug("BRK: Jump to: %x", reg.pc);
	//sleep(3);
	//exit(0);	
}

// NOP (denne hører egentlig ikke hjemme noe sted)
void NOP_(void) {	// 0xea
#ifdef DEBUG	
	c64_debug("        NOP");
#endif	
}

// Flag
void BIT_(void) {	// 0x2C
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
    c64_debug("BIT");
#endif
}

void BIT_abs(void) {	// 0x2C
	memAbsoluteAddr();
	BIT_();
}

void BIT_zp(void) {		// 0x24
	memZero();
	BIT_();
}

void CLC_(void) {		// 0x18
    // Clear Carry
    reg.p &= 0xfe;
#ifdef DEBUG
    c64_debug("        CLC");
#endif
}

void SEC_(void) {		// 0x38
    // Set Carry
    reg.p |= FLAG_C;
#ifdef DEBUG
    c64_debug("        SEC");
#endif
}

void CLI_(void) {		// 0x58
    // Clear Interrupt
    reg.p &= 0xfb;
#ifdef DEBUG
    c64_debug("        CLI");
#endif
}

void SEI_(void) {		// 0x78
    // Set Interrupt
    reg.p |= FLAG_I;
#ifdef DEBUG
    c64_debug("        SEI");
#endif
}

void CLV_(void) {		// 0xB8
    // Clear Overflow
    reg.p &= 0xbf;
#ifdef DEBUG
    c64_debug("        CLV");
#endif
}

void CLD_(void) {		// 0xD8
    // Clear Decimal
    reg.p &= 0xf7;
#ifdef DEBUG
    c64_debug("        CLD");
#endif
}

void SED_(void) {		// 0xF8
    // Clear Decimal
    reg.p |= FLAG_D;
#ifdef DEBUG
    c64_debug("        SED");
#endif
}

// Branching
void Branch_(void) {	// Wrapper for flere branch funksjoner
    memImm();
    // loadMem(effAddr);
#ifdef DEBUG
//    c64_debug("\tBranch: %02x (%d, sbyte: %02x (%d))\n", data, data, (signed char)data, (signed char)data);
#endif
	reg.pc += (signed char) data;
	reg.pc--;
}

void BPL_(void) {		// 0x10
    // Branch on N=0
#ifdef DEBUG
    c64_debug("        BPL");
#endif
    if (! (reg.p & FLAG_N)) Branch_();
	reg.pc++;
}

void BMI_(void) {		// 0x30
    // Branch on N=1
#ifdef DEBUG
    c64_debug("        BMI");
#endif
    if (reg.p & FLAG_N) Branch_();
	reg.pc++;
}

void BVC_(void) {		// 0x50
    // Branch on V=0	// FAAAAAAAAN!  Den kosta giljen en halv dag med arbeid
#ifdef DEBUG
    c64_debug("        BVC");
#endif
    if (! (reg.p & FLAG_V)) Branch_();
	reg.pc++;
}

void BVS_(void) {		// 0x70
    // branch on V=1
#ifdef DEBUG
    c64_debug("        BVS");
#endif
    if (reg.p & FLAG_V) Branch_();
	reg.pc++;
}

void BCC_(void) {		// 0x90
    // Branch on C=0
#ifdef DEBUG
    c64_debug("        BCC");
#endif
    if (! (reg.p & FLAG_C)) Branch_();
	reg.pc++;
}

void BCS_(void) {		// 0xB0
    // Branch on C=1
#ifdef DEBUG
    c64_debug("        BCS");
#endif
    if (reg.p & FLAG_C) Branch_();
	reg.pc++;
}

void BNE_(void) {		// 0xD0
    // Branch on Z=0
#ifdef DEBUG
    c64_debug("        BNE");
#endif
    if (! (reg.p & FLAG_Z)) Branch_();
	reg.pc++;
}

void BEQ_(void) {		// 0xF0
    // Branch on Z=1
#ifdef DEBUG
    c64_debug("        BEQ");
#endif
    if (reg.p & FLAG_Z) Branch_();
	reg.pc++;
}

void JMP_abs(void) {	// 0x4c
    memAbsoluteAddr();
		
	// sjekk om vi går inn i uendelig løkke
	if (effAddr == (reg.pc - 2)) {
		work = 0;
		// c64_debug("Infinite Loop\nEmulation Stopped\n");
		return;
	}

    reg.pc = effAddr;
	reg.pc--;

#ifdef DEBUG
    c64_debug("JMP");
#endif
}

void JMP_ind(void) {	// 0x6c
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
    c64_debug("JMP");
#endif
}


// Subroutines
void JSR_(void) {		// 0x20
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
	c64_debug("JSR");
#endif	

	reg.pc = jmpAddr;
	reg.pc--;
}

void RTI_(void) {		// 0x40
    unsigned char page;
	unsigned char offset;
	
	if (reg.s == 0xff) {
#ifdef DEBUG
//		c64_debug("Stack Overflow!\n");
//		c64_debug("Emulation Stopped\n");
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
	c64_debug("        RTI");
#endif

	// sjekk om vi er ferdige med et IRQ kall
	if (reg.s == 0xff) {
		work = 0;
	}
	
	return;
}

void RTS_(void) {		// 0x60
    unsigned char page;
	unsigned char offset;
	
	if (reg.s == 0xff) {
#ifdef DEBUG
//		c64_debug("Stack Overflow!\n");
//		c64_debug("Emulation Stopped\n");
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
	c64_debug("        RTS");
#endif
}

