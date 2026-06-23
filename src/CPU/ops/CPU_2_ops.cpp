#include "CPU.h"

uint8_t carry;

void CPU::set_half_carry_addition(uint8_t a8, uint8_t r8) {
	if ( ( (a8 & 0xF) + (r8 & 0xF) )  > 0xF ) {
		set_flag(HALF_CARRY_FLAG);
	} else {
		unset_flag(HALF_CARRY_FLAG);
	}
}

void CPU::set_half_carry_subtraction(uint8_t a8, uint8_t r8) {
	if ( (r8 & 0xF) > (a8 & 0xF) ) {
		set_flag(HALF_CARRY_FLAG);
	} else {
		unset_flag(HALF_CARRY_FLAG);
	}
}

void CPU::set_carry_addition(uint8_t a8, uint8_t r8) {
	if ( ( a8 + r8) > 0xFF ) {
		set_flag(CARRY_FLAG);
	} else {
		unset_flag(CARRY_FLAG);
	}
}

void CPU::set_carry_subtraction(uint8_t a8, uint8_t r8) {
	if ( r8 > a8 ) {
		set_flag(CARRY_FLAG);
	} else {
		unset_flag(CARRY_FLAG);
	}
}

void CPU::add_a(uint8_t r8) {
	uint8_t prev_a = A;
	A = (uint8_t)(A + r8);
	if (A == 0) { set_flag(ZERO_FLAG); } else { unset_flag(ZERO_FLAG); }
	unset_flag(SUBTRACTION_FLAG);
	set_half_carry_addition(prev_a, r8);
	set_carry_addition(prev_a, r8);
}

void CPU::adc_a(uint8_t r8) {
	uint8_t prev_a = A;
	A = (uint8_t)(A + r8 + get_flag(CARRY_FLAG));
	if (A == 0) { set_flag(ZERO_FLAG); } else { unset_flag(ZERO_FLAG); }
	unset_flag(SUBTRACTION_FLAG);
	if ( ( (prev_a & 0xF) + (r8 & 0xF) + get_flag(CARRY_FLAG) ) > 0xF) {
		set_flag(HALF_CARRY_FLAG);
	} else {
		unset_flag(HALF_CARRY_FLAG);
	}
	if ( ( (prev_a & 0xFF) + (r8 & 0xFF) + get_flag(CARRY_FLAG) ) > 0xFF) {
		set_flag(CARRY_FLAG);
	} else {
		unset_flag(CARRY_FLAG);
	}
}

void CPU::sub_a(uint8_t r8) {
	uint8_t prev_a = A;
	A = (uint8_t)(A - r8);
	if (A == 0) { set_flag(ZERO_FLAG); } else { unset_flag(ZERO_FLAG); }
	set_flag(SUBTRACTION_FLAG);
	set_half_carry_subtraction(prev_a, r8);
	set_carry_subtraction(prev_a, r8);
}

void CPU::sbc_a(uint8_t r8) {
	uint8_t prev_a = A;
	A = (uint8_t)(A - r8 - get_flag(CARRY_FLAG));
	if (A == 0) { set_flag(ZERO_FLAG); } else { unset_flag(ZERO_FLAG); }
	set_flag(SUBTRACTION_FLAG);
	if ( (r8 & 0xF) + get_flag(CARRY_FLAG) > (prev_a & 0xF) ) {
		set_flag(HALF_CARRY_FLAG);
	} else {
		unset_flag(HALF_CARRY_FLAG);
	}
	if ( (r8 & 0xFF) + get_flag(CARRY_FLAG) > (prev_a & 0xFF) ) {
		set_flag(CARRY_FLAG);
	} else {
		unset_flag(CARRY_FLAG);
	}
}

void CPU::and_a(uint8_t r8) {
	A = A & r8;
	if (A == 0) { set_flag(ZERO_FLAG); } else { unset_flag(ZERO_FLAG); }
	unset_flag(SUBTRACTION_FLAG);
	set_flag(HALF_CARRY_FLAG);
	unset_flag(CARRY_FLAG);
}

void CPU::xor_a(uint8_t r8) {
	A = A ^ r8;
	if (A == 0) { set_flag(ZERO_FLAG); } else { unset_flag(ZERO_FLAG); }
	unset_flag(SUBTRACTION_FLAG);
	unset_flag(HALF_CARRY_FLAG);
	unset_flag(CARRY_FLAG);
}

void CPU::or_a(uint8_t r8) {
	A = A | r8;
	if (A == 0) { set_flag(ZERO_FLAG); } else { unset_flag(ZERO_FLAG); }
	unset_flag(SUBTRACTION_FLAG);
	unset_flag(HALF_CARRY_FLAG);
	unset_flag(CARRY_FLAG);
}

void CPU::cp_a(uint8_t r8) {
	uint8_t prev_a = A;
	uint8_t cp_val = (uint8_t)(A - r8);
	if (cp_val == 0) { set_flag(ZERO_FLAG); } else { unset_flag(ZERO_FLAG); }
	set_flag(SUBTRACTION_FLAG);
	set_half_carry_subtraction(prev_a, r8);
	set_carry_subtraction(prev_a, r8);
}


void CPU::execute_2_ops(Opcode& opcode) {
	uint8_t r8 = A;
	uint8_t prev_carry = get_flag(CARRY_FLAG);

	switch (opcode.r) {
		case 0:  r8 = B ; break;
		case 1:  r8 = C ; break;
		case 2:  r8 = D ; break;
		case 3:  r8 = E ; break;
		case 4:  r8 = H ; break;
		case 5:  r8 = L ; break;
		case 6:  r8 = mmu.fetch(getHL()) ; opcode.tcyc = 8; break;
		default: r8 = A ; break;
	}

	switch (opcode.m) {
		case 0: add_a(r8) ; break; // ADD A,r8 ADD A,[HL]
		case 1: adc_a(r8) ; break; // ADC A,r8 ADC A,[HL]
		case 2: sub_a(r8) ; break; // SUB A,r8 SUB A,[HL]
		case 3: sbc_a(r8) ; break; // SBC A,r8 SBC A,[HL]
		case 4: and_a(r8) ; break; // AND A,r8 AND A,[HL]
		case 5: xor_a(r8) ; break; // XOR A,r8 XOR A,[HL]
		case 6:  or_a(r8) ; break; //  OR A,r8  OR A,[HL]
		case 7:  cp_a(r8) ; break; //  CP A,r8  CP A,[HL]
		default:            break;
	}

	if (opcode.r == 7) {
		if (opcode.m == 2) {
			set_flag(ZERO_FLAG);
			unset_flag(HALF_CARRY_FLAG);
			unset_flag(CARRY_FLAG);
		}
		if (opcode.m == 3) {
			if (prev_carry == 1) {
				set_flag(CARRY_FLAG);
			} else {
				unset_flag(CARRY_FLAG);
			}
		}
		if (opcode.m == 5) {
			set_flag(ZERO_FLAG);
			unset_flag(SUBTRACTION_FLAG);
			unset_flag(HALF_CARRY_FLAG);
			unset_flag(CARRY_FLAG);
		}
		if (opcode.m == 7) {
			set_flag(ZERO_FLAG);
			unset_flag(HALF_CARRY_FLAG);
			unset_flag(CARRY_FLAG);
		}
	}
}


/*
Flag behaviour:
Zero flag works as expected for all, except for XOR A, A and CP A, A
Subtraction flag is 0, except for SUB, SBC, CP

Half-carry flag works for all ADD, ADC, SUB (except SUB A A, but logically follows), SBC, and CP
But is set to 1 for all AND
And 0 for all OR and XOR

Carry flag works for ADD, ADC, SUB, SBC, and CP
But is not affected for SBC, A,A
Is set to zero for SUB A,A
And is zero for all AND, XOR, OR
*/