// Logiske og Aritmetiske Opkoder

// Compare - CMP/CPX/CPY
static void Compare_(unsigned char r, unsigned char d) {
    // tre funksjoner:
    // N(8) flag: settes hvis subtraksjon er negativ
    // Z(2) flag: ved likhet mellom verdiene (altsÂ, sub. == 0)
    // C(1) flag: hvis acc. er >= verdi
    unsigned char temp;

	if (d)
		loadMem(effAddr);

    temp = r - data;
#ifdef DEBUG
    platform_debug("CMP");
#endif
    evalNZ(temp);

    // "Carry will be set if the accumulator is equal to or greater than compared value"
    reg.p &= 0xfe;
    if (r >= data) reg.p |= FLAG_C;
}

static void CMP_imm(void) {	// 0xC9
    memImm();
	Compare_(reg.a, 0);
}

static void CMP_abs(void) {	// 0xCD
    memAbsoluteAddr();
    Compare_(reg.a, 1);
}

static void CMP_absx(void) {	// 0xDD
    memAbsoluteAddrX();
    Compare_(reg.a, 1);
}

static void CMP_absy(void) {	// 0xD9
    memAbsoluteAddrY();
    Compare_(reg.a, 1);
}

static void CMP_zp(void) {		// 0xC5
	memZero();
    Compare_(reg.a, 1);
}

static void CMP_zpx(void) {	// 0xD5
	memZeroX();
    Compare_(reg.a, 1);
}

static void CMP_izx(void) {	// 0xC1
	memIndirectZeroX();
	Compare_(reg.a, 1);
}

static void CMP_izy(void) {	// 0xD1
	memIndirectZeroY();
	Compare_(reg.a, 1);
}

static void CPX_imm(void) {	// 0xE0
	memImm();
	Compare_(reg.x, 0);
}

static void CPX_abs(void) {	// 0xEC
	memAbsoluteAddr();
	Compare_(reg.x, 1);
}

static void CPX_zp(void) {		// 0xE4
	memZero();
	Compare_(reg.x, 1);
}

static void CPY_imm(void) {	// 0xC0
	memImm();
	Compare_(reg.y, 0);
}

static void CPY_abs(void) {	// 0xCC
    memAbsoluteAddr();
    Compare_(reg.y, 1);
}

static void CPY_zp(void) {		// 0xC4
    memZero();
    Compare_(reg.y, 1);
}

static void DEC_(void) {
    // dekrementer adressa med 1
    loadMem(effAddr);
    data--;
    evalNZ(data);
#ifdef DEBUG
    platform_debug("DEC  [%04x] {%02x}", effAddr, data);
#endif
    storeMem(data);
}

static void DEC_abs(void) {	// 0xCE
    memAbsoluteAddr();
    DEC_();
}

static void DEC_absx(void) {	// 0xDE
    memAbsoluteAddrX();
    DEC_();
}

static void DEC_zp(void) {		// 0xC6
    memZero();
    DEC_();
}

static void DEC_zpx(void) {	// 0xD6
    memZeroX();
    DEC_();
}

static void INC_(void) {
    // inkrementer adressa med 1
    loadMem(effAddr);
    data++;
    evalNZ(data);
    storeMem(data);
#ifdef DEBUG
    platform_debug("INC");
#endif
}

static void INC_abs(void) {	// 0xEE
	memAbsoluteAddr();
	INC_();
}

static void INC_absx(void) {	// 0xFE
    memAbsoluteAddrX();
    INC_();    
}

static void INC_zp(void) {		// 0xE6
	memZero();
	INC_();
}

static void INC_zpx(void) {	// 0xF6
	memZeroX();
	INC_();
}

static void INX_(void) {		// 0xE8
    reg.x++;
#ifdef DEBUG
    platform_debug("        INX");
#endif
    evalNZ(reg.x);
}

static void DEX_(void) {		// 0xCA
    reg.x--;
#ifdef DEBUG
    platform_debug("        DEX");
#endif
    evalNZ(reg.x);
}

static void INY_(void) {		// 0xC8
    reg.y++;
#ifdef DEBUG
    platform_debug("        INY");
#endif
    evalNZ(reg.y);
}

static void DEY_(void) {		// 0x88
    reg.y--;
#ifdef DEBUG
    platform_debug("        DEY");
#endif
    evalNZ(reg.y);
}

static void ASL_(void) {
    // left shift (bit 7 gÂr til C)
	loadMem(effAddr);
    reg.p &= 0xfe;
    if (data & 0x80) reg.p |= 0x1;
    data <<= 1;
    evalNZ(data);
	storeMem(data);
#ifdef DEBUG
    platform_debug("        ASL");
#endif
}

static void ASL_abs(void) {	// 0x0E
	memAbsoluteAddr();
	ASL_();
}

static void ASL_absx(void) {	// 0x1E
	memAbsoluteAddrX();
	ASL_();
}

static void ASL_zp(void) {		// 0x06
	memZero();
	ASL_();
}

static void ASL_zpx(void) {	// 0x16
	memZeroX();
	ASL_();
}

static void ASL_imp(void) {	// 0x0A
    reg.p &= 0xfe;
    if (reg.a & 0x80) reg.p |= 0x1;
    reg.a <<= 1;
    evalNZ(reg.a);
#ifdef DEBUG
    platform_debug("ASL");
#endif
}

static void LSR_(void) {
	loadMem(effAddr);
    reg.p &= 0xfe;
    if (data & 0x1) reg.p |= 0x1;
    data >>= 1;
    evalNZ(data);
#ifdef DEBUG
    platform_debug("        LSR");
#endif
    storeMem(data);
}

static void LSR_abs(void) {	// 0x4E
	memAbsoluteAddr();
	LSR_();
}

static void LSR_absx(void) {	// 0x5E
	memAbsoluteAddrX();
	LSR_();
}

static void LSR_zp(void) {		// 0x46
	memZero();
	LSR_();
}

static void LSR_zpx(void) {	// 0x56
	memZeroX();
	LSR_();
}

static void LSR_imp(void) {	// 0x4A
    // right shift (bit 0 gÂr til C)
    reg.p &= 0xfe;
    if (reg.a & 0x1) reg.p |= 0x1;
    reg.a >>= 1;
    evalNZ(reg.a);
#ifdef DEBUG
    platform_debug("        LSR");
#endif
}

// ROTATE
static void ROL_(void) {
    // left rotate
    loadMem(effAddr);
    unsigned char temp = reg.p & 0x1;

    reg.p &= 0xfe;
    if (data & 0x80) reg.p |= 0x1;

    data <<= 1;
    if (temp) data |= 0x1;
    evalNZ(data);
    storeMem(data);
#ifdef DEBUG
    platform_debug("        ROL");
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
    platform_debug("        ROL");
#endif
}

static void ROL_abs(void) {	// 0x2E
	memAbsoluteAddr();
	ROL_();
}

static void ROL_absx(void) {	// 0x3E
	memAbsoluteAddrX();
	ROL_();
}

static void ROL_zp(void) {		// 0x26
    memZero();
    ROL_();    
}

static void ROL_zpx(void) {	// 0x36
	memZeroX();
	ROL_();
}

static void ROR_(void) {
    // right rotate
    loadMem(effAddr);
    unsigned char temp = reg.p & 0x1;

    reg.p &= 0xfe;
    if (data & 0x1) reg.p |= 0x1;

    data >>= 1;
    if (temp) data |= 0x80;
    evalNZ(data);
	storeMem(data);
#ifdef DEBUG
    platform_debug("ROR");
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
    platform_debug("        ROR");
#endif
}

static void ROR_abs(void) {	// 0x6E
	memAbsoluteAddr();
	ROR_();
}

static void ROR_absx(void) {	// 0x7E
	memAbsoluteAddrX();
	ROR_();
}

static void ROR_zp(void) {		// 0x66
    memZero();
    ROR_();    
}

static void ROR_zpx(void) {	// 0x76
    memZeroX();
    ROR_();    
}

// AND
static void AND_(void) {
    loadMem(effAddr);
#ifdef DEBUG
    platform_debug("AND");
#endif
    reg.a &= data;
    evalNZ(reg.a);
}

static void AND_imm(void) {	// 0x29
    memImm();
#ifdef DEBUG
    platform_debug("AND_imm");
#endif
    reg.a &= data;
    evalNZ(reg.a);
}

static void AND_abs(void) {	// 0x2D
	memAbsoluteAddr();
	AND_();
}

static void AND_absx(void) {	// 0x3D
    memAbsoluteAddrX();
    AND_();
}

static void AND_absy(void) {	// 0x39
	memAbsoluteAddrY();
	AND_();
}

static void AND_zp(void) {		// 0x25
	memZero();
	AND_();
}

static void AND_zpx(void) {	// 0x35
	memZeroX();
	AND_();
}

static void AND_izx(void) {	// 0x21
	memIndirectZeroX();
	AND_();
}

static void AND_izy(void) {	// 0x31
	memIndirectZeroY();
	AND_();
}

// OR(acc)
static void ORA_(void) {
    loadMem(effAddr);
#ifdef DEBUG
    platform_debug("ORA");
#endif
    reg.a |= data;
    evalNZ(reg.a);
}

static void ORA_imm(void) {	// 0x09
	memImm();
#ifdef DEBUG
    platform_debug("ORA_imm");
#endif
    reg.a |= data;
    evalNZ(reg.a);
}

static void ORA_abs(void) {	// 0x0D
	memAbsoluteAddr();
	ORA_();
}

static void ORA_absx(void) {	// 0x1D
	memAbsoluteAddrX();
	ORA_();
}

static void ORA_absy(void) {	// 0x19
	memAbsoluteAddrY();
	ORA_();
}

static void ORA_zp(void) {		// 0x05
	memZero();
	ORA_();
}

static void ORA_zpx(void) {	// 0x15
	memZero();
	ORA_();
}

static void ORA_izx(void) {	// 0x01
	memIndirectZeroX();
	ORA_();
}

static void ORA_izy(void) {	// 0x11
	memIndirectZeroY();
	ORA_();
}

// EOR(acc)
static void EOR_(void) {
    loadMem(effAddr);
#ifdef DEBUG
    platform_debug("EOR");
#endif
    reg.a ^= data;
    evalNZ(reg.a);
}

static void EOR_imm(void) {	// 0x49
	memImm();
#ifdef DEBUG
    platform_debug("EOR_imm");
#endif
    reg.a ^= data;
    evalNZ(reg.a);
}

static void EOR_abs(void) {	// 0x4D
	memAbsoluteAddr();
	EOR_();
}

static void EOR_absx(void) {	// 0x5D
	memAbsoluteAddrX();
	EOR_();
}

static void EOR_absy(void) {	// 0x59
	memAbsoluteAddrY();
	EOR_();
}

static void EOR_zp(void) {		// 0x45
	memZero();
	EOR_();
}

static void EOR_zpx(void) {	// 0x55
	memZeroX();
	EOR_();
}

static void EOR_izx(void) {	// 0x41
	memIndirectZeroX();
	EOR_();
}

static void EOR_izy(void) {	// 0x51
	memIndirectZeroY();
	EOR_();
}

// ADC og SBC baserer seg veldig sterkt pÂ SIDPlayer-4.4
// ADC, ADD with Carry
static void ADC_(unsigned char d) {
	if (d)
		loadMem(effAddr);
		
    unsigned char carry = reg.p & 0x1;
    
    if (reg.p & FLAG_D) {
	// BCD calc.
	platform_abort("ADC: BCD calculation is not yet implemented\n");
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
	platform_debug("ADC");
#endif
	reg.a = (temp & 0xff);
	evalNZ(reg.a);
    }
}

static void ADC_imm(void) {	// 0x69
    memImm();
    ADC_(0);
}

static void ADC_abs(void) {	// 0x6D
    memAbsoluteAddr();
    ADC_(1);
}

static void ADC_absx(void) {	// 0x7D
    memAbsoluteAddrX();
    ADC_(1);
}

static void ADC_absy(void) {	// 0x79
    memAbsoluteAddrY();
    ADC_(1);
}

static void ADC_zp(void) {		// 0x65
    memZero();
    ADC_(1);
}

static void ADC_zpx(void) {	// 0x75
	memZeroX();
	ADC_(1);
}

static void ADC_izx(void) {	// 0x61
	memIndirectZeroX();
	ADC_(1);
}

static void ADC_izy(void) {	// 0x71
	memIndirectZeroY();
	ADC_(1);
}


// SBC, Subtract with Carry
static void SBC_(unsigned char d) {
	if (d)
		loadMem(effAddr);

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
	platform_abort("SBC: BCD calculation is not yet implemented\n");
    } else {
	// Binary
#ifdef DEBUG
	platform_debug("SBC");
#endif
	reg.a = (temp & 0xff);
    }
    evalNZ(reg.a);
}

static void SBC_imm(void) {	// 0xE9
    memImm();
    SBC_(0);
}

static void SBC_abs(void) {	// 0xED
    memAbsoluteAddr();
    SBC_(1);
}

static void SBC_absx(void) {	// 0xFD
    memAbsoluteAddrX();
    SBC_(1);
}

static void SBC_absy(void) {	// 0xF9
    memAbsoluteAddrY();
    SBC_(1);
}

static void SBC_zp(void) {		// 0xE5
    memZero();
    SBC_(1);
}

static void SBC_zpx(void) {	// 0xF5
    memZeroX();
    SBC_(1);
}

static void SBC_izx(void) {	// 0xE1
    memIndirectZeroX();
    SBC_(1);
}

static void SBC_izy(void) {	// 0xF1
    memIndirectZeroY();
    SBC_(1);
}

