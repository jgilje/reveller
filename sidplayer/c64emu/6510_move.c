// Load/Transfer/Store/Stack

// LOAD
// Accumulator
static void LDA_(void) {
    c64_loadMem(effAddr);
    reg.a = data;
    evalNZ(reg.a);
#ifdef DEBUG
    platform_debug("LDA   [%04x]", effAddr);
#endif
}

static void LDA_imm(void) {	// 0xA9
    c64_memImm();
    reg.a = data;
    evalNZ(reg.a);
#ifdef DEBUG
    platform_debug("LDA_imm");
#endif
}

static void LDA_abs(void) {	// 0xAD
    c64_memAbsoluteAddr();
    LDA_();
}

static void LDA_absx(void) {	// 0xBD
    c64_memAbsoluteAddrX();
    LDA_();
}

static void LDA_absy(void) {	// 0xB9
    c64_memAbsoluteAddrY();
    LDA_();
}

static void LDA_zp(void) {		// 0xA5
    c64_memZero();
    LDA_();
}

static void LDA_zpx(void) {	// 0xB5
        c64_memZeroX();
	LDA_();
}

static void LDA_izx(void) {	// 0xA1
        c64_memIndirectZeroX();
	LDA_();
}

static void LDA_izy(void) {	// 0xB1
    c64_memIndirectZeroY();
    LDA_();
}

// RegX
static void LDX_(void) {
    c64_loadMem(effAddr);
    reg.x = data;
#ifdef DEBUG
    platform_debug("LDX");
#endif
    evalNZ(reg.x);
}

static void LDX_imm(void) {	// 0xA2
    c64_memImm();
	reg.x = data;
#ifdef DEBUG
    platform_debug("LDX_imm");
#endif
    evalNZ(reg.x);
}

static void LDX_abs(void) {	// 0xAE
    c64_memAbsoluteAddr();
	LDX_();
}

static void LDX_absy(void) {	// 0xAE
    c64_memAbsoluteAddrY();
	LDX_();
}

static void LDX_zp(void) {		// 0xA6
    c64_memZero();
	LDX_();
}

static void LDX_zpy(void) {	// 0xB6
    c64_memZeroY();
	LDX_();    
}

// RegY
static void LDY_(void) {
    c64_loadMem(effAddr);
    reg.y = data;
    evalNZ(reg.y);
#ifdef DEBUG
    platform_debug("LDY");
#endif
}

static void LDY_imm(void) {	// 0xA0
    c64_memImm();
    reg.y = data;
    evalNZ(reg.y);
#ifdef DEBUG
    platform_debug("LDY_imm");
#endif
}

static void LDY_abs(void) {	// 0xAC
    c64_memAbsoluteAddr();
    LDY_();
}

static void LDY_absx(void) {	// 0xBC
    c64_memAbsoluteAddrX();
    LDY_();
}

static void LDY_zp(void) {		// 0xA4
    c64_memZero();
    LDY_();
}

static void LDY_zpx(void) {	// 0xB4
        c64_memZeroX();
	LDY_();
}

// STA
static void STA_abs(void) {	// 0x8D
    c64_memAbsoluteAddr();
#ifdef DEBUG
    platform_debug("STAa  [%04x]", effAddr);
#endif    
    c64_storeMem(reg.a);
}

static void STA_absx(void) {	// 0x9D
    c64_memAbsoluteAddrX();
#ifdef DEBUG
    platform_debug("STAax [%04x]", effAddr);
#endif    
    c64_storeMem(reg.a);
}

static void STA_absy(void) {	// 0x99
    c64_memAbsoluteAddrY();
#ifdef DEBUG
    platform_debug("STAay [%04x]", effAddr);
#endif    
    c64_storeMem(reg.a);
}

static void STA_zp(void) {		// 0x85
    c64_memZero();
#ifdef DEBUG
    platform_debug("STAz  [%04x]", effAddr);
#endif
    c64_storeMem(reg.a);
}

static void STA_zpx(void) {	// 0x95
    c64_memZeroX();
#ifdef DEBUG
    platform_debug("STAzx [%04x]", effAddr);
#endif
    c64_storeMem(reg.a);
}

static void STA_izx(void) {	// 0x81
    c64_memIndirectZeroX();
#ifdef DEBUG
    platform_debug("STAix [%04x]", effAddr);
#endif
    c64_storeMem(reg.a);
}

static void STA_izy(void) {	// 0x91
    c64_memIndirectZeroY();
#ifdef DEBUG
    platform_debug("STAiy [%04x]", effAddr);
#endif
    c64_storeMem(reg.a);
}

// STX
static void STX_abs(void) {	// 0x8E
    c64_memAbsoluteAddr();
#ifdef DEBUG
    platform_debug("STXa");
#endif
    c64_storeMem(reg.x);
}

static void STX_zp(void) {		// 0x86
    c64_memZero();
#ifdef DEBUG
    platform_debug("STXz");
#endif    
    c64_storeMem(reg.x);
}

static void STX_zpy(void) {	// 0x96
    c64_memZeroY();
#ifdef DEBUG
    platform_debug("STXzy");
#endif    
    c64_storeMem(reg.x);
}

// STY
static void STY_abs(void) {	// 0x8C
    c64_memAbsoluteAddr();
#ifdef DEBUG
    platform_debug("STYa");
#endif    
    c64_storeMem(reg.y);
}

static void STY_zp(void) {		// 0x84
    c64_memZero();
#ifdef DEBUG
    platform_debug("STYz");
#endif    
    c64_storeMem(reg.y);
}

static void STY_zpx(void) {	// 0x94
    c64_memZeroX();
#ifdef DEBUG
    platform_debug("STYzx");
#endif    
    c64_storeMem(reg.y);
}

// Transfer
static void TAX_(void) {		// 0xAA
    reg.x = reg.a;
    evalNZ(reg.x);
#ifdef DEBUG
    platform_debug("        TAX");
#endif
    
}

static void TXA_(void) {		// 0x8A
    reg.a = reg.x;
    evalNZ(reg.a);
#ifdef DEBUG
    platform_debug("        TXA");
#endif
    
}

static void TAY_(void) {		// 0xA8
    reg.y = reg.a;
    evalNZ(reg.y);
#ifdef DEBUG
    platform_debug("        TAY");
#endif
    
}

static void TYA_(void) {		// 0x98
    reg.a = reg.y;
    evalNZ(reg.a);
#ifdef DEBUG
    platform_debug("        TYA");
#endif
    
}

static void TXS_(void) {		// 0x9A
	reg.s = reg.x;
#ifdef DEBUG
    platform_debug("        TXS");
#endif    
}

static void TSX_(void) {		// 0xBA
    reg.x = reg.s;
    evalNZ(reg.x);
#ifdef DEBUG
    platform_debug("        TSX");
#endif    
}

// Stack Instructions
static void PushStack(unsigned char d) {
	if (reg.s == 0x00) {
#ifdef DEBUG
//		platform_debug(" Stack Overflow! ");
#endif
		work = 0;
		return;
    }
	
    c64_memStack();
    c64_storeMem(d);
    reg.s--;
}

static void PullStack(void) {
    if (reg.s == 0xff) {
#ifdef DEBUG
//		platform_debug(" Stack Overflow! ");
#endif
		work = 0;
		return;
    }

    reg.s++;
    c64_memStack();
    c64_loadMem(effAddr);
}

static void PHA_(void) {		// 0x48
	PushStack(reg.a);
#ifdef DEBUG
    platform_debug("        PHA");
#endif    
}

static void PLA_(void) {		// 0x48
	PullStack();
    reg.a = data;
    evalNZ(reg.a);
#ifdef DEBUG
    platform_debug("        PLA");
#endif    
}

static void PHP_(void) {		// 0x08
	PushStack(reg.p);
#ifdef DEBUG
    platform_debug("        PHP");
#endif    
}

static void PLP_(void) {		// 0x28
	PullStack();
    reg.p = data;
#ifdef DEBUG
    platform_debug("        PLP");
#endif    
}

