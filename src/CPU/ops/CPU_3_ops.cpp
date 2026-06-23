#include "CPU.h"

void CPU::pushn16(uint16_t n16) {
	uint8_t msb = (n16 >> 8);
	uint8_t lsb = n16 & 0xFF;
	setSP(getSP() - 1);
	mmu.set(getSP(), msb);
	setSP(getSP() - 1);
	mmu.set(getSP(), lsb);
}

uint16_t CPU::popn16() {
	uint8_t lsb = mmu.fetch(getSP());
	setSP(getSP() + 1);
	uint8_t msb = mmu.fetch(getSP());
	setSP(getSP() + 1);
	return (uint16_t)((msb << 8) + lsb);
}

void CPU::calla16(uint16_t n16) {
	mmu.m_tick();
	pushn16(PC + 3); // as call operations are 3-bytes
	PC = n16;
	jump_flag = true;
}

void CPU::rst(uint16_t n16) {
	mmu.m_tick();
	pushn16(PC + 1); // as call operations are 3-bytes
	PC = n16;
	jump_flag = true;
}

void CPU::execute_3_ops(Opcode& opcode) {
	uint8_t e8 = 0;
	uint16_t n16 = 0;
	switch(opcode.r) {
	case 0:
		opcode.tcyc = 8;
		if (opcode.m < 4) {
			mmu.m_tick();
			if (opcode.m == 0 && get_flag(ZERO_FLAG) == 0) { // RET NZ
				opcode.tcyc = 20;
				opcode.sz = 0;
				PC = popn16();
				mmu.m_tick();
			}
			if (opcode.m == 1 && get_flag(ZERO_FLAG) == 1) { // RET Z
				opcode.tcyc = 20;
				opcode.sz = 0;
				PC = popn16();
				mmu.m_tick();
			}
			if (opcode.m == 2 && get_flag(CARRY_FLAG) == 0) { // RET NC
				opcode.tcyc = 20;
				opcode.sz = 0;
				PC = popn16();
				mmu.m_tick();
			}
			if (opcode.m == 3 && get_flag(CARRY_FLAG) == 1) { // RET C
				opcode.tcyc = 20;
				opcode.sz = 0;
				PC = popn16();
				mmu.m_tick();
			}
		} else {
			switch(opcode.m) {
			case 4: // LDH [a8],A
				opcode.sz = 2; opcode.tcyc = 12;
				mmu.set((uint16_t)(0xFF00 + PC1()), A);
				break; 
			case 5: // ADD SP,e8
				opcode.sz = 2; opcode.tcyc = 16;
				unset_flag(ZERO_FLAG);
				unset_flag(SUBTRACTION_FLAG);
				e8 = PC1();
				set_half_carry_addition(SP & 0xFF, e8);
				set_carry_addition(SP & 0xFF, e8);
				setSP((uint16_t)(getSP() + convert_e8_to_decimal(e8)));
				mmu.m_tick();
				mmu.m_tick();
				break;
			case 6: // LDH A,[a8]
				opcode.sz = 2; opcode.tcyc = 12;
				A = mmu.fetch((uint16_t)(0xFF00 + PC1()));
				break;
			case 7: // LD HL,SP+e8
				opcode.sz = 2; opcode.tcyc = 12;
				e8 = PC1();
				unset_flag(ZERO_FLAG);
				unset_flag(SUBTRACTION_FLAG);
				set_half_carry_addition(SP & 0xFF, e8);
				set_carry_addition(SP & 0xFF, e8);
				setHL((uint16_t)(getSP() + convert_e8_to_decimal(e8)));
				mmu.m_tick();
				break;
			default: break;
			}
		}
		break;
	case 1:
		switch(opcode.m) {
			case 0: setBC(popn16()); opcode.tcyc = 12; break; // POP BC
			case 2: setDE(popn16()); opcode.tcyc = 12; break; // POP DE
			case 4: setHL(popn16()); opcode.tcyc = 12; break; // POP HL
			case 6: setAF(popn16()); opcode.tcyc = 12; break; // POP AF

			case 1: PC = popn16(); mmu.m_tick(); opcode.tcyc = 16; opcode.sz = 0; break; // RET
			case 3: PC = popn16(); mmu.m_tick(); opcode.tcyc = 16; opcode.sz = 0; IME = 1; break; // RETI
			case 5: opcode.sz = 0; PC = getHL(); break; // JP HL
			case 7: setSP(getHL()); opcode.tcyc = 8; mmu.m_tick(); break; // LD SP,HL
			default:
				break;
		}
		break;
	case 2:
		switch(opcode.m) {
		case 0: // JP NZ, a16
		case 1: // JP  Z, a16
		case 2: // JP NC, a16
		case 3: // JP  C, a16
			opcode.sz = 3; opcode.tcyc = 12;
			n16 = (PC2() << 8) + PC1();
			if (opcode.m == 0 && get_flag(ZERO_FLAG) == 0) {
				opcode.tcyc = 16;
				opcode.sz = 0;
				PC = n16;
				mmu.m_tick();
			}
			if (opcode.m == 1 && get_flag(ZERO_FLAG) == 1) {
				opcode.tcyc = 16;
				opcode.sz = 0;
				PC = n16;
				mmu.m_tick();
			}
			if (opcode.m == 2 && get_flag(CARRY_FLAG) == 0) {
				opcode.tcyc = 16;
				opcode.sz = 0;
				PC = n16;
				mmu.m_tick();
			}
			if (opcode.m == 3 && get_flag(CARRY_FLAG) == 1) {
				opcode.tcyc = 16;
				opcode.sz = 0;
				PC = n16;
				mmu.m_tick();
			}
			break;
		case 4: // LDH [C],A
			opcode.tcyc = 8;
			mmu.set((uint16_t)(0xFF00 + C), A);
			break;
		case 5: // LD [a16],A
			opcode.tcyc = 16; opcode.sz = 3;
			mmu.set((uint16_t)( (PC2() << 8) + PC1() ), A);
			break;
		case 6: // LDH A,[C]
			opcode.tcyc = 8;
			A = mmu.fetch((uint16_t)(0xFF00 + C));
			break;
		case 7: // LD A,[a16]
			opcode.tcyc = 16; opcode.sz = 3;
			A = mmu.fetch((uint16_t)( (PC2() << 8) + PC1() ));
			break;
		default:
			break;
		}
		break;
	case 3:
		switch(opcode.m) {
			case 0: // JP a16
				PC = (PC2() << 8) + PC1();
				mmu.m_tick();
				opcode.sz = 0; opcode.tcyc = 16;
				break;
			case 1: // PREFIX (CB)
				opcode.sz = 1; opcode.tcyc = 4; // I assume CB and t-cycle of instruction add together to get overall t-cyc value?
				opcode = extract_lmr(PC1());
				execute_cb_ops(opcode);
				break;
			case 6: IME = 0; break; // DI
			case 7: set_IME = 1; break; // EI use set_IME flag to delay effect of IME by one instruction
			default:
				break;
		}
		break;
	case 4:
		opcode.tcyc = 12; opcode.sz = 3;
		n16 = (uint16_t)((PC2() << 8) + PC1());
		if (opcode.m == 0 && get_flag(ZERO_FLAG) == 0) { // CALL NZ,a16
			opcode.tcyc = 24;
			opcode.sz = 0;
			calla16(n16);
		}
		if (opcode.m == 1 && get_flag(ZERO_FLAG) == 1) { // CALL Z,a16
			opcode.tcyc = 24;
			opcode.sz = 0;
			calla16(n16);
		}
		if (opcode.m == 2 && get_flag(CARRY_FLAG) == 0) { // CALL NC,a16
			opcode.tcyc = 24;
			opcode.sz = 0;
			calla16(n16);
		}
		if (opcode.m == 3 && get_flag(CARRY_FLAG) == 1) { // CALL C,a16
			opcode.tcyc = 24;
			opcode.sz = 0;
			calla16(n16);	
		}
		break;
	case 5:
		opcode.tcyc = 16;
		switch(opcode.m) {
			case 0: mmu.m_tick(); pushn16(getBC()); break; // PUSH BC
			case 2: mmu.m_tick(); pushn16(getDE()); break; // PUSH DE
			case 4: mmu.m_tick(); pushn16(getHL()); break; // PUSH HL
			case 6: mmu.m_tick(); pushn16(getAF()); break; // PUSH AF
			case 1: // CALL a16
				calla16((uint16_t)((PC2() << 8) + PC1()));
				opcode.sz = 0; opcode.tcyc = 24; 
				break;
			default: break;
		}
		break;
	case 6:
		opcode.sz = 2; opcode.tcyc = 8;
		switch (opcode.m) {
		case 0: add_a(PC1()); break; // ADD A,n8
		case 1: adc_a(PC1()); break; // ADC A,n8
		case 2: sub_a(PC1()); break; // SUB A,n8
		case 3: sbc_a(PC1()); break; // SBC A,n8
		case 4: and_a(PC1()); break; // AND A,n8
		case 5: xor_a(PC1()); break; // XOR A,n8
		case 6:  or_a(PC1()); break; //  OR A,n8
		case 7:  cp_a(PC1()); break; //  CP A,n8
		default:            break;
		}
		break;
	case 7:
		opcode.tcyc = 16;
		opcode.sz = 0;
		switch (opcode.m) {
		case 0:  rst(0x0000); break; // RST $00
		case 1:  rst(0x0008); break; // RST $08
		case 2:  rst(0x0010); break; // RST $10
		case 3:  rst(0x0018); break; // RST $18
		case 4:  rst(0x0020); break; // RST $20
		case 5:  rst(0x0028); break; // RST $28
		case 6:  rst(0x0030); break; // RST $30
		case 7:  rst(0x0038); break; // RST $38
		default:                  break;
		}
		break;
	default:
		break;
	}
	return;
}