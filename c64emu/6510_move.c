// Load/Transfer/Store/Stack

// LOAD
// Accumulator
void LDA_(void) {
    loadMem(effAddr);
    reg.a = data;
    evalNZ(reg.a);
#ifdef DEBUG
    rprintf("LDA   [%04x]", effAddr);
#endif
}

void LDA_imm(void) {	// 0xA9
    memImm();
    reg.a = data;
    evalNZ(reg.a);
#ifdef DEBUG
    rprintf("LDA_imm");
#endif
}

void LDA_abs(void) {	// 0xAD
    memAbsoluteAddr();
    LDA_();
}

void LDA_absx(void) {	// 0xBD
    memAbsoluteAddrX();
    LDA_();
}

void LDA_absy(void) {	// 0xB9
    memAbsoluteAddrY();
    LDA_();
}

void LDA_zp(void) {		// 0xA5
    memZero();
    LDA_();
}

void LDA_zpx(void) {	// 0xB5
	memZeroX();
	LDA_();
}

void LDA_izx(void) {	// 0xA1
	memIndirectZeroX();
	LDA_();
}

void LDA_izy(void) {	// 0xB1
    memIndirectZeroY();
    LDA_();
}

// RegX
void LDX_(void) {
    loadMem(effAddr);
    reg.x = data;
#ifdef DEBUG
    rprintf("LDX");
#endif
    evalNZ(reg.x);
}

void LDX_imm(void) {	// 0xA2
    memImm();
	reg.x = data;
#ifdef DEBUG
    rprintf("LDX_imm");
#endif
    evalNZ(reg.x);
}

void LDX_abs(void) {	// 0xAE
    memAbsoluteAddr();
	LDX_();
}

void LDX_absy(void) {	// 0xAE
    memAbsoluteAddrY();
	LDX_();
}

void LDX_zp(void) {		// 0xA6
    memZero();
	LDX_();
}

void LDX_zpy(void) {	// 0xB6
    memZeroY();
	LDX_();    
}

// RegY
void LDY_(void) {
    loadMem(effAddr);
    reg.y = data;
    evalNZ(reg.y);
#ifdef DEBUG
    rprintf("LDY");
#endif
}

void LDY_imm(void) {	// 0xA0
    memImm();
    reg.y = data;
    evalNZ(reg.y);
#ifdef DEBUG
    rprintf("LDY_imm");
#endif
}

void LDY_abs(void) {	// 0xAC
    memAbsoluteAddr();
    LDY_();
}

void LDY_absx(void) {	// 0xBC
    memAbsoluteAddrX();
    LDY_();
}

void LDY_zp(void) {		// 0xA4
    memZero();
    LDY_();
}

void LDY_zpx(void) {	// 0xB4
	memZeroX();
	LDY_();
}

// STA
void STA_abs(void) {	// 0x8D
    memAbsoluteAddr();
#ifdef DEBUG
    rprintf("STAa  [%04x]", effAddr);
#endif    
    storeMem(reg.a);
}

void STA_absx(void) {	// 0x9D
    memAbsoluteAddrX();
#ifdef DEBUG
    rprintf("STAax [%04x]", effAddr);
#endif    
    storeMem(reg.a);
}

void STA_absy(void) {	// 0x99
    memAbsoluteAddrY();
#ifdef DEBUG
    rprintf("STAay [%04x]", effAddr);
#endif    
    storeMem(reg.a);
}

void STA_zp(void) {		// 0x85
    memZero();
#ifdef DEBUG
    rprintf("STAz  [%04x]", effAddr);
#endif
    storeMem(reg.a);
}

void STA_zpx(void) {	// 0x95
    memZeroX();
#ifdef DEBUG
    rprintf("STAzx [%04x]", effAddr);
#endif
    storeMem(reg.a);
}

void STA_izx(void) {	// 0x81
    memIndirectZeroX();
#ifdef DEBUG
    rprintf("STAix [%04x]", effAddr);
#endif
    storeMem(reg.a);
}

void STA_izy(void) {	// 0x91
    memIndirectZeroY();
#ifdef DEBUG
    rprintf("STAiy [%04x]", effAddr);
#endif
    storeMem(reg.a);
}

// STX
void STX_abs(void) {	// 0x8E
    memAbsoluteAddr();
#ifdef DEBUG
    rprintf("STXa");
#endif
    storeMem(reg.x);
}

void STX_zp(void) {		// 0x86
    memZero();
#ifdef DEBUG
    rprintf("STXz");
#endif    
    storeMem(reg.x);
}

void STX_zpy(void) {	// 0x96
    memZeroY();
#ifdef DEBUG
    rprintf("STXzy");
#endif    
    storeMem(reg.x);
}

// STY
void STY_abs(void) {	// 0x8C
    memAbsoluteAddr();
#ifdef DEBUG
    rprintf("STYa");
#endif    
    storeMem(reg.y);
}

void STY_zp(void) {		// 0x84
    memZero();
#ifdef DEBUG
    rprintf("STYz");
#endif    
    storeMem(reg.y);
}

void STY_zpx(void) {	// 0x94
    memZeroX();
#ifdef DEBUG
    rprintf("STYzx");
#endif    
    storeMem(reg.y);
}

// Transfer
void TAX_(void) {		// 0xAA
    reg.x = reg.a;
    evalNZ(reg.x);
#ifdef DEBUG
    rprintf("        TAX");
#endif
    
}

void TXA_(void) {		// 0x8A
    reg.a = reg.x;
    evalNZ(reg.a);
#ifdef DEBUG
    rprintf("        TXA");
#endif
    
}

void TAY_(void) {		// 0xA8
    reg.y = reg.a;
    evalNZ(reg.y);
#ifdef DEBUG
    rprintf("        TAY");
#endif
    
}

void TYA_(void) {		// 0x98
    reg.a = reg.y;
    evalNZ(reg.a);
#ifdef DEBUG
    rprintf("        TYA");
#endif
    
}

void TXS_(void) {		// 0x9A
	reg.s = reg.x;
#ifdef DEBUG
    rprintf("        TXS");
#endif    
}

void TSX_(void) {		// 0xBA
    reg.x = reg.s;
    evalNZ(reg.x);
#ifdef DEBUG
    rprintf("        TSX");
#endif    
}

// Stack Instructions
void PushStack(unsigned char d) {
	if (reg.s == 0x00) {
#ifdef DEBUG
//		rprintf(" Stack Overflow! ");
#endif
		work = 0;
		return;
    }
	
    memStack();
    storeMem(d);
    reg.s--;
}

void PullStack(void) {
    if (reg.s == 0xff) {
#ifdef DEBUG
//		rprintf(" Stack Overflow! ");
#endif
		work = 0;
		return;
    }

    reg.s++;
    memStack();
    loadMem(effAddr);
}

void PHA_(void) {		// 0x48
	PushStack(reg.a);
#ifdef DEBUG
    rprintf("        PHA");
#endif    
}

void PLA_(void) {		// 0x48
	PullStack();
    reg.a = data;
    evalNZ(reg.a);
#ifdef DEBUG
    rprintf("        PLA");
#endif    
}

void PHP_(void) {		// 0x08
	PushStack(reg.p);
#ifdef DEBUG
    rprintf("        PHP");
#endif    
}

void PLP_(void) {		// 0x28
	PullStack();
    reg.p = data;
#ifdef DEBUG
    rprintf("        PLP");
#endif    
}

