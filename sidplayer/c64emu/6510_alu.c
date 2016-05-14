// Logiske og Aritmetiske Opkoder

// Compare - CMP/CPX/CPY
static void Compare_(unsigned char r, unsigned char d) {
    // tre funksjoner:
    // N(8) flag: settes hvis subtraksjon er negativ
    // Z(2) flag: ved likhet mellom verdiene (altsÂ, sub. == 0)
    // C(1) flag: hvis acc. er >= verdi
    unsigned char temp;

	if (d)
                c64_loadMem(effAddr);

    temp = r - data;
#ifdef DEBUG
    reveller->debug("CMP");
#endif
    evalNZ(temp);

    // "Carry will be set if the accumulator is equal to or greater than compared value"
    reg.p &= 0xfe;
    if (r >= data) reg.p |= FLAG_C;
}

static void CMP_imm(void) {	// 0xC9
    c64_memImm();
	Compare_(reg.a, 0);
}

static void CMP_abs(void) {	// 0xCD
    c64_memAbsoluteAddr();
    Compare_(reg.a, 1);
}

static void CMP_absx(void) {	// 0xDD
    c64_memAbsoluteAddrX();
    Compare_(reg.a, 1);
}

static void CMP_absy(void) {	// 0xD9
    c64_memAbsoluteAddrY();
    Compare_(reg.a, 1);
}

static void CMP_zp(void) {		// 0xC5
        c64_memZero();
    Compare_(reg.a, 1);
}

static void CMP_zpx(void) {	// 0xD5
        c64_memZeroX();
    Compare_(reg.a, 1);
}

static void CMP_izx(void) {	// 0xC1
        c64_memIndirectZeroX();
	Compare_(reg.a, 1);
}

static void CMP_izy(void) {	// 0xD1
        c64_memIndirectZeroY();
	Compare_(reg.a, 1);
}

static void CPX_imm(void) {	// 0xE0
        c64_memImm();
	Compare_(reg.x, 0);
}

static void CPX_abs(void) {	// 0xEC
        c64_memAbsoluteAddr();
	Compare_(reg.x, 1);
}

static void CPX_zp(void) {		// 0xE4
        c64_memZero();
	Compare_(reg.x, 1);
}

static void CPY_imm(void) {	// 0xC0
        c64_memImm();
	Compare_(reg.y, 0);
}

static void CPY_abs(void) {	// 0xCC
    c64_memAbsoluteAddr();
    Compare_(reg.y, 1);
}

static void CPY_zp(void) {		// 0xC4
    c64_memZero();
    Compare_(reg.y, 1);
}

static void DEC_(void) {
    // dekrementer adressa med 1
    c64_loadMem(effAddr);
    data--;
    evalNZ(data);
#ifdef DEBUG
    reveller->debug("DEC  [%04x] {%02x}", effAddr, data);
#endif
    c64_storeMem(data);
}

static void DEC_abs(void) {	// 0xCE
    c64_memAbsoluteAddr();
    DEC_();
}

static void DEC_absx(void) {	// 0xDE
    c64_memAbsoluteAddrX();
    DEC_();
}

static void DEC_zp(void) {		// 0xC6
    c64_memZero();
    DEC_();
}

static void DEC_zpx(void) {	// 0xD6
    c64_memZeroX();
    DEC_();
}

static void INC_(void) {
    // inkrementer adressa med 1
    c64_loadMem(effAddr);
    data++;
    evalNZ(data);
    c64_storeMem(data);
#ifdef DEBUG
    reveller->debug("INC");
#endif
}

static void INC_abs(void) {	// 0xEE
        c64_memAbsoluteAddr();
	INC_();
}

static void INC_absx(void) {	// 0xFE
    c64_memAbsoluteAddrX();
    INC_();    
}

static void INC_zp(void) {		// 0xE6
        c64_memZero();
	INC_();
}

static void INC_zpx(void) {	// 0xF6
        c64_memZeroX();
	INC_();
}

static void INX_(void) {		// 0xE8
    reg.x++;
#ifdef DEBUG
    reveller->debug("        INX");
#endif
    evalNZ(reg.x);
}

static void DEX_(void) {		// 0xCA
    reg.x--;
#ifdef DEBUG
    reveller->debug("        DEX");
#endif
    evalNZ(reg.x);
}

static void INY_(void) {		// 0xC8
    reg.y++;
#ifdef DEBUG
    reveller->debug("        INY");
#endif
    evalNZ(reg.y);
}

static void DEY_(void) {		// 0x88
    reg.y--;
#ifdef DEBUG
    reveller->debug("        DEY");
#endif
    evalNZ(reg.y);
}

static void ASL_(void) {
    // left shift (bit 7 gÂr til C)
        c64_loadMem(effAddr);
    reg.p &= 0xfe;
    if (data & 0x80) reg.p |= 0x1;
    data <<= 1;
    evalNZ(data);
        c64_storeMem(data);
#ifdef DEBUG
    reveller->debug("        ASL");
#endif
}

static void ASL_abs(void) {	// 0x0E
        c64_memAbsoluteAddr();
	ASL_();
}

static void ASL_absx(void) {	// 0x1E
        c64_memAbsoluteAddrX();
	ASL_();
}

static void ASL_zp(void) {		// 0x06
        c64_memZero();
	ASL_();
}

static void ASL_zpx(void) {	// 0x16
        c64_memZeroX();
	ASL_();
}

static void ASL_imp(void) {	// 0x0A
    reg.p &= 0xfe;
    if (reg.a & 0x80) reg.p |= 0x1;
    reg.a <<= 1;
    evalNZ(reg.a);
#ifdef DEBUG
    reveller->debug("ASL");
#endif
}

static void LSR_(void) {
        c64_loadMem(effAddr);
    reg.p &= 0xfe;
    if (data & 0x1) reg.p |= 0x1;
    data >>= 1;
    evalNZ(data);
#ifdef DEBUG
    reveller->debug("        LSR");
#endif
    c64_storeMem(data);
}

static void LSR_abs(void) {	// 0x4E
        c64_memAbsoluteAddr();
	LSR_();
}

static void LSR_absx(void) {	// 0x5E
        c64_memAbsoluteAddrX();
	LSR_();
}

static void LSR_zp(void) {		// 0x46
        c64_memZero();
	LSR_();
}

static void LSR_zpx(void) {	// 0x56
        c64_memZeroX();
	LSR_();
}

static void LSR_imp(void) {	// 0x4A
    // right shift (bit 0 gÂr til C)
    reg.p &= 0xfe;
    if (reg.a & 0x1) reg.p |= 0x1;
    reg.a >>= 1;
    evalNZ(reg.a);
#ifdef DEBUG
    reveller->debug("        LSR");
#endif
}

// ROTATE
static void ROL_(void) {
    // left rotate
    c64_loadMem(effAddr);
    unsigned char temp = reg.p & 0x1;

    reg.p &= 0xfe;
    if (data & 0x80) reg.p |= 0x1;

    data <<= 1;
    if (temp) data |= 0x1;
    evalNZ(data);
    c64_storeMem(data);
#ifdef DEBUG
    reveller->debug("        ROL");
#endif
}

static void ROL_imp(void) {		// 0x2A
    // left rotate
    unsigned char temp = reg.p & 0x1;

    reg.p &= 0xfe;
    if (reg.a & 0x80) reg.p |= 0x1;

    reg.a <<= 1;
    if (temp) reg.a |= 0x1;
    evalNZ(reg.a);
#ifdef DEBUG
    reveller->debug("        ROL");
#endif
}

static void ROL_abs(void) {	// 0x2E
        c64_memAbsoluteAddr();
	ROL_();
}

static void ROL_absx(void) {	// 0x3E
        c64_memAbsoluteAddrX();
	ROL_();
}

static void ROL_zp(void) {		// 0x26
    c64_memZero();
    ROL_();    
}

static void ROL_zpx(void) {	// 0x36
        c64_memZeroX();
	ROL_();
}

static void ROR_(void) {
    // right rotate
    c64_loadMem(effAddr);
    unsigned char temp = reg.p & 0x1;

    reg.p &= 0xfe;
    if (data & 0x1) reg.p |= 0x1;

    data >>= 1;
    if (temp) data |= 0x80;
    evalNZ(data);
        c64_storeMem(data);
#ifdef DEBUG
    reveller->debug("ROR");
#endif
}

static void ROR_imp(void) {	// 0x6A
    unsigned char temp = reg.p & 0x1;

    reg.p &= 0xfe;
    if (reg.a & 0x1) reg.p |= 0x1;

    reg.a >>= 1;
    if (temp) reg.a |= 0x80;
    evalNZ(reg.a);
#ifdef DEBUG
    reveller->debug("        ROR");
#endif
}

static void ROR_abs(void) {	// 0x6E
        c64_memAbsoluteAddr();
	ROR_();
}

static void ROR_absx(void) {	// 0x7E
        c64_memAbsoluteAddrX();
	ROR_();
}

static void ROR_zp(void) {		// 0x66
    c64_memZero();
    ROR_();    
}

static void ROR_zpx(void) {	// 0x76
    c64_memZeroX();
    ROR_();    
}

// AND
static void AND_(void) {
    c64_loadMem(effAddr);
#ifdef DEBUG
    reveller->debug("AND");
#endif
    reg.a &= data;
    evalNZ(reg.a);
}

static void AND_imm(void) {	// 0x29
    c64_memImm();
#ifdef DEBUG
    reveller->debug("AND_imm");
#endif
    reg.a &= data;
    evalNZ(reg.a);
}

static void AND_abs(void) {	// 0x2D
        c64_memAbsoluteAddr();
	AND_();
}

static void AND_absx(void) {	// 0x3D
    c64_memAbsoluteAddrX();
    AND_();
}

static void AND_absy(void) {	// 0x39
        c64_memAbsoluteAddrY();
	AND_();
}

static void AND_zp(void) {		// 0x25
        c64_memZero();
	AND_();
}

static void AND_zpx(void) {	// 0x35
        c64_memZeroX();
	AND_();
}

static void AND_izx(void) {	// 0x21
        c64_memIndirectZeroX();
	AND_();
}

static void AND_izy(void) {	// 0x31
        c64_memIndirectZeroY();
	AND_();
}

// OR(acc)
static void ORA_(void) {
    c64_loadMem(effAddr);
#ifdef DEBUG
    reveller->debug("ORA");
#endif
    reg.a |= data;
    evalNZ(reg.a);
}

static void ORA_imm(void) {	// 0x09
        c64_memImm();
#ifdef DEBUG
    reveller->debug("ORA_imm");
#endif
    reg.a |= data;
    evalNZ(reg.a);
}

static void ORA_abs(void) {	// 0x0D
        c64_memAbsoluteAddr();
	ORA_();
}

static void ORA_absx(void) {	// 0x1D
        c64_memAbsoluteAddrX();
	ORA_();
}

static void ORA_absy(void) {	// 0x19
        c64_memAbsoluteAddrY();
	ORA_();
}

static void ORA_zp(void) {		// 0x05
        c64_memZero();
	ORA_();
}

static void ORA_zpx(void) {	// 0x15
        c64_memZero();
	ORA_();
}

static void ORA_izx(void) {	// 0x01
        c64_memIndirectZeroX();
	ORA_();
}

static void ORA_izy(void) {	// 0x11
        c64_memIndirectZeroY();
	ORA_();
}

// EOR(acc)
static void EOR_(void) {
    c64_loadMem(effAddr);
#ifdef DEBUG
    reveller->debug("EOR");
#endif
    reg.a ^= data;
    evalNZ(reg.a);
}

static void EOR_imm(void) {	// 0x49
        c64_memImm();
#ifdef DEBUG
    reveller->debug("EOR_imm");
#endif
    reg.a ^= data;
    evalNZ(reg.a);
}

static void EOR_abs(void) {	// 0x4D
        c64_memAbsoluteAddr();
	EOR_();
}

static void EOR_absx(void) {	// 0x5D
        c64_memAbsoluteAddrX();
	EOR_();
}

static void EOR_absy(void) {	// 0x59
        c64_memAbsoluteAddrY();
	EOR_();
}

static void EOR_zp(void) {		// 0x45
        c64_memZero();
	EOR_();
}

static void EOR_zpx(void) {	// 0x55
        c64_memZeroX();
	EOR_();
}

static void EOR_izx(void) {	// 0x41
        c64_memIndirectZeroX();
	EOR_();
}

static void EOR_izy(void) {	// 0x51
        c64_memIndirectZeroY();
	EOR_();
}

// ADC og SBC baserer seg veldig sterkt pÂ SIDPlayer-4.4
// ADC, ADD with Carry
static void ADC_(unsigned char d) {
	if (d)
                c64_loadMem(effAddr);
		
    unsigned char carry = reg.p & 0x1;
    
    if (reg.p & FLAG_D) {
	// BCD calc.
        reveller->abort("ADC: BCD calculation is not yet implemented\n");
    } else {
	// Binary
	unsigned int temp = reg.a + data + carry;
	
	// alt untatt bit 7 (V) og bit 1 (C)
	// (0xff ^ FLAG_C ^ FLAG_V);
	reg.p &= 0xbe;
	// Sett Carry og Overflow
	if (temp > 0xff) { 
	    reg.p |= FLAG_C;
	}
	
	if (!((reg.a ^ data) & 0x80) && ((reg.a ^ temp) & 0x80)) {
		reg.p |= FLAG_V;
	}
#ifdef DEBUG
        reveller->debug("ADC");
#endif
	reg.a = (temp & 0xff);
	evalNZ(reg.a);
    }
}

static void ADC_imm(void) {	// 0x69
    c64_memImm();
    ADC_(0);
}

static void ADC_abs(void) {	// 0x6D
    c64_memAbsoluteAddr();
    ADC_(1);
}

static void ADC_absx(void) {	// 0x7D
    c64_memAbsoluteAddrX();
    ADC_(1);
}

static void ADC_absy(void) {	// 0x79
    c64_memAbsoluteAddrY();
    ADC_(1);
}

static void ADC_zp(void) {		// 0x65
    c64_memZero();
    ADC_(1);
}

static void ADC_zpx(void) {	// 0x75
        c64_memZeroX();
	ADC_(1);
}

static void ADC_izx(void) {	// 0x61
        c64_memIndirectZeroX();
	ADC_(1);
}

static void ADC_izy(void) {	// 0x71
        c64_memIndirectZeroY();
	ADC_(1);
}


// SBC, Subtract with Carry
static void SBC_(unsigned char d) {
	if (d)
                c64_loadMem(effAddr);

    unsigned char carry = !(reg.p & 0x1);
    unsigned int temp = reg.a - data - carry;

    // regn ut FLAG_V og FLAG_C
    reg.p &= 0xbe;
    if (((reg.a ^ temp) & 0x80) && ((reg.a ^ data) & 0x80)) {
		reg.p |= FLAG_V;
	}
    if (temp < 0x100) {
		reg.p |= FLAG_C;
    }    
    
    if (reg.p & FLAG_D) {
	// BCD calc
        reveller->abort("SBC: BCD calculation is not yet implemented\n");
    } else {
	// Binary
#ifdef DEBUG
        reveller->debug("SBC");
#endif
	reg.a = (temp & 0xff);
    }
    evalNZ(reg.a);
}

static void SBC_imm(void) {	// 0xE9
    c64_memImm();
    SBC_(0);
}

static void SBC_abs(void) {	// 0xED
    c64_memAbsoluteAddr();
    SBC_(1);
}

static void SBC_absx(void) {	// 0xFD
    c64_memAbsoluteAddrX();
    SBC_(1);
}

static void SBC_absy(void) {	// 0xF9
    c64_memAbsoluteAddrY();
    SBC_(1);
}

static void SBC_zp(void) {		// 0xE5
    c64_memZero();
    SBC_(1);
}

static void SBC_zpx(void) {	// 0xF5
    c64_memZeroX();
    SBC_(1);
}

static void SBC_izx(void) {	// 0xE1
    c64_memIndirectZeroX();
    SBC_(1);
}

static void SBC_izy(void) {	// 0xF1
    c64_memIndirectZeroY();
    SBC_(1);
}

