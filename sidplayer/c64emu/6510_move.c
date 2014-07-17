// Load/Transfer/Store/Stack

// LOAD
// Accumulator
void LDA_(void) {
    loadMem(effAddr);
    reg.a = data;
    evalNZ(reg.a);
#ifdef DEBUG
    platform_debug("LDA   [%04x]", effAddr);
#endif
}

void LDA_imm(void) {	// 0xA9
    memImm();
    reg.a = data;
    evalNZ(reg.a);
#ifdef DEBUG
    platform_debug("LDA_imm");
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
    platform_debug("LDX");
#endif
    evalNZ(reg.x);
}

void LDX_imm(void) {	// 0xA2
    memImm();
	reg.x = data;
#ifdef DEBUG
    platform_debug("LDX_imm");
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
    platform_debug("LDY");
#endif
}

void LDY_imm(void) {	// 0xA0
    memImm();
    reg.y = data;
    evalNZ(reg.y);
#ifdef DEBUG
    platform_debug("LDY_imm");
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
    platform_debug("STAa  [%04x]", effAddr);
#endif    
    storeMem(reg.a);
}

void STA_absx(void) {	// 0x9D
    memAbsoluteAddrX();
#ifdef DEBUG
    platform_debug("STAax [%04x]", effAddr);
#endif    
    storeMem(reg.a);
}

void STA_absy(void) {	// 0x99
    memAbsoluteAddrY();
#ifdef DEBUG
    platform_debug("STAay [%04x]", effAddr);
#endif    
    storeMem(reg.a);
}

void STA_zp(void) {		// 0x85
    memZero();
#ifdef DEBUG
    platform_debug("STAz  [%04x]", effAddr);
#endif
    storeMem(reg.a);
}

void STA_zpx(void) {	// 0x95
    memZeroX();
#ifdef DEBUG
    platform_debug("STAzx [%04x]", effAddr);
#endif
    storeMem(reg.a);
}

void STA_izx(void) {	// 0x81
    memIndirectZeroX();
#ifdef DEBUG
    platform_debug("STAix [%04x]", effAddr);
#endif
    storeMem(reg.a);
}

void STA_izy(void) {	// 0x91
    memIndirectZeroY();
#ifdef DEBUG
    platform_debug("STAiy [%04x]", effAddr);
#endif
    storeMem(reg.a);
}

// STX
void STX_abs(void) {	// 0x8E
    memAbsoluteAddr();
#ifdef DEBUG
    platform_debug("STXa");
#endif
    storeMem(reg.x);
}

void STX_zp(void) {		// 0x86
    memZero();
#ifdef DEBUG
    platform_debug("STXz");
#endif    
    storeMem(reg.x);
}

void STX_zpy(void) {	// 0x96
    memZeroY();
#ifdef DEBUG
    platform_debug("STXzy");
#endif    
    storeMem(reg.x);
}

// STY
void STY_abs(void) {	// 0x8C
    memAbsoluteAddr();
#ifdef DEBUG
    platform_debug("STYa");
#endif    
    storeMem(reg.y);
}

void STY_zp(void) {		// 0x84
    memZero();
#ifdef DEBUG
    platform_debug("STYz");
#endif    
    storeMem(reg.y);
}

void STY_zpx(void) {	// 0x94
    memZeroX();
#ifdef DEBUG
    platform_debug("STYzx");
#endif    
    storeMem(reg.y);
}

// Transfer
void TAX_(void) {		// 0xAA
    reg.x = reg.a;
    evalNZ(reg.x);
#ifdef DEBUG
    platform_debug("        TAX");
#endif
    
}

void TXA_(void) {		// 0x8A
    reg.a = reg.x;
    evalNZ(reg.a);
#ifdef DEBUG
    platform_debug("        TXA");
#endif
    
}

void TAY_(void) {		// 0xA8
    reg.y = reg.a;
    evalNZ(reg.y);
#ifdef DEBUG
    platform_debug("        TAY");
#endif
    
}

void TYA_(void) {		// 0x98
    reg.a = reg.y;
    evalNZ(reg.a);
#ifdef DEBUG
    platform_debug("        TYA");
#endif
    
}

void TXS_(void) {		// 0x9A
	reg.s = reg.x;
#ifdef DEBUG
    platform_debug("        TXS");
#endif    
}

void TSX_(void) {		// 0xBA
    reg.x = reg.s;
    evalNZ(reg.x);
#ifdef DEBUG
    platform_debug("        TSX");
#endif    
}

// Stack Instructions
void PushStack(unsigned char d) {
	if (reg.s == 0x00) {
#ifdef DEBUG
//		platform_debug(" Stack Overflow! ");
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
//		platform_debug(" Stack Overflow! ");
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
    platform_debug("        PHA");
#endif    
}

void PLA_(void) {		// 0x48
	PullStack();
    reg.a = data;
    evalNZ(reg.a);
#ifdef DEBUG
    platform_debug("        PLA");
#endif    
}

void PHP_(void) {		// 0x08
	PushStack(reg.p);
#ifdef DEBUG
    platform_debug("        PHP");
#endif    
}

void PLP_(void) {		// 0x28
	PullStack();
    reg.p = data;
#ifdef DEBUG
    platform_debug("        PLP");
#endif    
}

