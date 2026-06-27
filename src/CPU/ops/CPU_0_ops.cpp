#include "CPU.h"

void CPU::set_16_bit_half_carry_addition(uint16_t a, uint16_t b) {
	if ((int)( (a & 0xFFF) + (b & 0xFFF) ) > 0xFFF) {
		set_flag(HALF_CARRY_FLAG);
	} else {
		unset_flag(HALF_CARRY_FLAG);
	}
}

void CPU::set_16_bit_carry_addition(uint16_t a, uint16_t b) {
	if ((int)(a + b) > 0xFFFF) {
		set_flag(CARRY_FLAG);
	} else {
		unset_flag(CARRY_FLAG);
	}
}
				
void CPU::rlca() {
	uint8_t bit7 = A >> 7;
	A = A << 1;
	A = A | bit7;
	if (bit7 == 1) {
		set_flag(CARRY_FLAG);
	} else {
		unset_flag(CARRY_FLAG);
	}
	unset_flag(ZERO_FLAG);
	unset_flag(SUBTRACTION_FLAG);
	unset_flag(HALF_CARRY_FLAG);
}

void CPU::rrca() {
	uint8_t bit0 = A & 0b1;
	A = A >> 1;
	A = A | (bit0 << 7);
	if (bit0 == 1) {
		set_flag(CARRY_FLAG);
	} else {
		unset_flag(CARRY_FLAG);
	}
	unset_flag(ZERO_FLAG);
	unset_flag(SUBTRACTION_FLAG);
	unset_flag(HALF_CARRY_FLAG);
}

void CPU::rla() {
	uint8_t bit7 = A >> 7;
	uint8_t prev_carry = get_flag(CARRY_FLAG);
	A = A << 1;
	A = A | prev_carry;
	if (bit7 == 1) {
		set_flag(CARRY_FLAG);
	} else {
		unset_flag(CARRY_FLAG);
	}
	unset_flag(ZERO_FLAG);
	unset_flag(SUBTRACTION_FLAG);
	unset_flag(HALF_CARRY_FLAG);
} 

void CPU::rra() {
	uint8_t bit0 = A & 0b1;
	uint8_t prev_carry = get_flag(CARRY_FLAG);
	A = (A >> 1) & 0x7F;
	A = A | (prev_carry << 7);
	if (bit0 == 1) {
		set_flag(CARRY_FLAG);
	} else {
		unset_flag(CARRY_FLAG);
	}
	unset_flag(ZERO_FLAG);
	unset_flag(SUBTRACTION_FLAG);
	unset_flag(HALF_CARRY_FLAG);
}

void CPU::daa() {
	uint8_t adjustment = 0;
	if (get_flag(SUBTRACTION_FLAG) == 1) {
		if (get_flag(HALF_CARRY_FLAG) == 1) {
			adjustment += 0x6;
		}
		if (get_flag(CARRY_FLAG) == 1) {
			adjustment += 0x60;
		}
		A -= adjustment;
	} else {
		if (get_flag(HALF_CARRY_FLAG) == 1 || ( (A & 0xF) > 0x9)) {
			adjustment += 0x6;
		}
		if (get_flag(CARRY_FLAG) == 1 || A > 0x99) {
			adjustment += 0x60;
			set_flag(CARRY_FLAG);
		}
		A += adjustment;
	}
	if (A == 0) {
		set_flag(ZERO_FLAG);
	} else {
		unset_flag(ZERO_FLAG);
	}
	unset_flag(HALF_CARRY_FLAG);
}

void CPU::cpl() {
	set_flag(SUBTRACTION_FLAG);
	set_flag(HALF_CARRY_FLAG);
	A = ~A;
}

void CPU::scf() {
	unset_flag(SUBTRACTION_FLAG);
	unset_flag(HALF_CARRY_FLAG);
	set_flag(CARRY_FLAG);
}

void CPU::ccf() {
	unset_flag(SUBTRACTION_FLAG);
	unset_flag(HALF_CARRY_FLAG);
	if (get_flag(CARRY_FLAG) == 1) {
		unset_flag(CARRY_FLAG);
	} else {
		set_flag(CARRY_FLAG);
	}
}

void CPU::execute_0_ops(Opcode& opcode) {
	uint8_t* ptr = &A;
	uint8_t e8 = 0;
	uint16_t n16 = 0;
	switch (opcode.r) {
		case 0:
			switch (opcode.m) {
				case 0: opcode.sz = 1; opcode.tcyc = 4; break; // NOP
				case 1: // LD [a16],SP
					n16 = (PC2() << 8) + PC1();
					mmu.set(n16, getSP() & 0xFF );
					mmu.set(n16 + 1, getSP() >> 8);
					opcode.tcyc = 20;
					opcode.sz = 3;
					break;
				case 2:
					opcode.tcyc = 4;
					opcode.sz = 2;
					mmu.set(KEY1, mmu.fetch(KEY1, false) | 0x80, false);
					mmu.m_tick();
					break;
				case 3: opcode.tcyc = 12; opcode.sz = 2; PC = PC + convert_e8_to_decimal(PC1()); mmu.m_tick(); break;
				case 4:
					opcode.tcyc = 8; opcode.sz = 2;
					e8 = PC1();
					if (get_flag(ZERO_FLAG) == 0) {
						opcode.sz = 0;
						opcode.tcyc = 12;
						PC = PC + 2 + convert_e8_to_decimal(e8);
						mmu.m_tick();
					}
					break;
				case 5:
					opcode.tcyc = 8; opcode.sz = 2;
					e8 = PC1();
					if (get_flag(ZERO_FLAG) == 1) {
						opcode.sz = 0;
						opcode.tcyc = 12;
						PC = PC + 2 + convert_e8_to_decimal(e8);
						mmu.m_tick();
					}
					break;
				case 6:
					opcode.tcyc = 8; opcode.sz = 2;
					e8 = PC1();
					if (get_flag(CARRY_FLAG) == 0) {
						opcode.sz = 0;
						opcode.tcyc = 12;
						PC = PC + 2 + convert_e8_to_decimal(e8);
						mmu.m_tick();
					}
					break;
				case 7:
					opcode.tcyc = 8; opcode.sz = 2;
					e8 = PC1();
					if (get_flag(CARRY_FLAG) == 1) {
						opcode.sz = 0;
						opcode.tcyc = 12;
						PC = PC + 2 + convert_e8_to_decimal(e8);
						mmu.m_tick();
					}
					break;
				default: break;
			}
			break;
		case 1:
			if (opcode.m % 2 == 0) {
				opcode.sz = 3;
				opcode.tcyc = 12;
				switch (opcode.m) {
					case 0:  setBC((uint16_t)((PC2() << 8) + PC1())); break; // LD BC,n16
					case 2:  setDE((uint16_t)((PC2() << 8) + PC1())); break; // LD DE,n16
					case 4:  setHL((uint16_t)((PC2() << 8) + PC1())); break; // LD HL,n16
					case 6:  setSP((uint16_t)((PC2() << 8) + PC1())); break; // LD SP,n16
					default: break;
				}
			} else {
				opcode.sz = 1;
				opcode.tcyc = 8;
				
				uint16_t r16 = 0;
				switch(opcode.m) {
				case 1: r16 = getBC(); mmu.m_tick(); break; // ADD HL,BC
				case 3: r16 = getDE(); mmu.m_tick();  break; // ADD HL,DE
				case 5: r16 = getHL(); mmu.m_tick();  break; // ADD HL,HL
				case 7: r16 = getSP(); mmu.m_tick();  break; // ADD HL,SP
				default: break;
				}

				set_16_bit_half_carry_addition(getHL(), r16);
				set_16_bit_carry_addition(getHL(), r16);
				unset_flag(SUBTRACTION_FLAG);
				setHL((uint16_t)(getHL() + r16));
				
			}
			break;
		case 2:
			opcode.tcyc = 8;
			switch(opcode.m) {
				case 0:  mmu.set(getBC(), A)    ; break; // LD [BC],A
				case 2:  mmu.set(getDE(), A)    ; break; // LD [DE],A
				case 1:  A = mmu.fetch(getBC()) ; break; // LD A,[BC]
				case 3:  A = mmu.fetch(getDE()) ; break; // LD A,[DE]

				case 4:  mmu.set(getHL(), A)    ; setHL(getHL() + 1); break; // LD [HL+],A
				case 6:  mmu.set(getHL(), A)    ; setHL(getHL() - 1); break; // LD [HL-],A
				case 5:  A = mmu.fetch(getHL()) ; setHL(getHL() + 1); break; // LD A,[HL+]
				case 7:  A = mmu.fetch(getHL()) ; setHL(getHL() - 1); break; // LD A,[HL-]
				default:                                              break; 
			}
			break;
		case 3:
			opcode.tcyc = 8;
			switch(opcode.m) {
				case 0:  setBC(getBC() + 1); break; // INC BC
				case 1:  setBC(getBC() - 1); break; // DEC BC
				case 2:  setDE(getDE() + 1); break; // INC DE
				case 3:  setDE(getDE() - 1); break; // DEC DE
				case 4:  setHL(getHL() + 1); break; // INC HL
				case 5:  setHL(getHL() - 1); break; // DEC HL
				case 6:  setSP(getSP() + 1); break; // INC SP
				case 7:  setSP(getSP() - 1); break; // DEC SP
				default:                     break;
			}
			mmu.m_tick();
			break;
		case 4:
		case 5:
			if (opcode.m == 6) {
				opcode.tcyc = 12;
				uint8_t r8 = mmu.fetch(getHL());
				if (opcode.r == 4) {
					unset_flag(SUBTRACTION_FLAG);
					set_half_carry_addition(r8, 1);
					r8 = r8 + 1;
					mmu.set(getHL(), r8); // INC [HL]
				} else {
					set_flag(SUBTRACTION_FLAG);
					set_half_carry_subtraction(r8, 1);
					r8 = r8 - 1;
					mmu.set(getHL(), r8); // DEC [HL]
				}
				if (r8 == 0) {
					set_flag(ZERO_FLAG);
				} else {
					unset_flag(ZERO_FLAG);
				}
			} else {
				ptr = &A; // Pointer was used as referent of reference cannot be changed after initialisation
				switch (opcode.m) {
					case 0:  ptr = &B; break; // INC B DEC B
					case 1:  ptr = &C; break; // INC C DEC C
					case 2:  ptr = &D; break; // INC D DEC D
					case 3:  ptr = &E; break; // INC E DEC E
					case 4:  ptr = &H; break; // INC H DEC H
					case 5:  ptr = &L; break; // INC L DEC L
					default: ptr = &A; break; // INC A DEC A
				}
				if (opcode.r == 4) {
					unset_flag(SUBTRACTION_FLAG);
					set_half_carry_addition(*ptr, 1);
					*ptr += 1;
				} else {
					set_flag(SUBTRACTION_FLAG);
					set_half_carry_subtraction(*ptr, 1);
					*ptr -= 1;
				}
				if (*ptr == 0) {
					set_flag(ZERO_FLAG);
				} else {
					unset_flag(ZERO_FLAG);
				}
			}
			break;
		case 6:
			opcode.sz = 2; opcode.tcyc = 8;
			switch(opcode.m) {
				case 0: B = PC1(); break; // LD B,n8
				case 1: C = PC1(); break; // LD C,n8
				case 2: D = PC1(); break; // LD D,n8
				case 3: E = PC1(); break; // LD E,n8
				case 4: H = PC1(); break; // LD H,n8
				case 5: L = PC1(); break; // LD L,n8
				case 6: mmu.set(getHL(), PC1()); opcode.tcyc = 12; break; // LD [HL],n8
				case 7: A = PC1(); break;  // LD A,n8
				default: break;
			}
			break;
		case 7:
			switch (opcode.m) {
				case 0:  rlca(); break; // RLCA
				case 1:  rrca(); break; // RRCA
				case 2:   rla(); break; // RLA
				case 3:   rra(); break; // RRA
				case 4:   daa(); break; // DAA
				case 5:   cpl(); break; // CPL
				case 6:   scf(); break; // SCF
				case 7:   ccf(); break; // CCF
				default:         break;
			}
			break;
		default:
			break;
	}
}
