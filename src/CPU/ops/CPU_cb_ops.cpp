#include "CPU.h"

uint8_t CPU::rlc(uint8_t r8) {
	uint8_t bit7 = r8 >> 7;
	if (bit7 == 1) {
		set_flag(CARRY_FLAG);
	} else {
		unset_flag(CARRY_FLAG);
	}
	r8 = r8 << 1;
	r8 = r8 | bit7;
	return r8;
}

uint8_t CPU::rrc(uint8_t r8) {
	uint8_t bit0 = r8 & 0b1;
	if (bit0 == 1) {
		set_flag(CARRY_FLAG);
	} else {
		unset_flag(CARRY_FLAG);
	}
	r8 = r8 >> 1;
	r8 = r8 | (bit0 << 7);
	return r8;
}

uint8_t CPU::rl(uint8_t r8) {
	uint8_t bit7 = r8 >> 7;
	r8 = r8 << 1;
	r8 = r8 | get_flag(CARRY_FLAG);
	if (bit7 == 1) {
		set_flag(CARRY_FLAG);
	} else {
		unset_flag(CARRY_FLAG);
	}
	return r8;
}

uint8_t CPU::rr(uint8_t r8) {
	uint8_t bit0 = r8 & 0b1;
	r8 = r8 >> 1;
	r8 = r8 | (get_flag(CARRY_FLAG) << 7);
	if (bit0 == 1) {
		set_flag(CARRY_FLAG);
	} else {
		unset_flag(CARRY_FLAG);
	}
	return r8;
}

uint8_t CPU::sla(uint8_t r8) {
	uint8_t bit7 = r8 >> 7;
	r8 = r8 << 1;
	if (bit7 == 1) {
		set_flag(CARRY_FLAG);
	} else {
		unset_flag(CARRY_FLAG);
	}
	return r8;
}

uint8_t CPU::sra(uint8_t r8) {
	uint8_t bit0 = r8 & 0b1;
	uint8_t bit7 = (r8 >> 7);
	r8 = r8 >> 1;
	r8 = r8 | (bit7 << 7);
	if (bit0 == 1) {
		set_flag(CARRY_FLAG);
	} else {
		unset_flag(CARRY_FLAG);
	}
	return r8;
}

uint8_t CPU::swap(uint8_t r8) {
	uint8_t msb = (r8 & 0xF0) >> 4;
	uint8_t lsb = r8 & 0xF;
	unset_flag(CARRY_FLAG);
	return (uint8_t)( (lsb << 4) + msb );
}

uint8_t CPU::srl(uint8_t r8) {
	uint8_t bit0 = r8 & 0b1;
	r8 = r8 >> 1;
	r8 = r8 & 0x7F;
	if (bit0 == 1) {
		set_flag(CARRY_FLAG);
	} else {
		unset_flag(CARRY_FLAG);
	}
	return r8;	
}

void CPU::execute_cb_ops(Opcode& opcode) {
	uint8_t sz_increment = 2; // DEPRECATED THESE (WILL CLEAN LATER)
	uint8_t tcyc_increment = 8; // DEPRECATED THESE (WILL CLEAN LATER)
	uint8_t* val = &A;
	uint8_t val_noref = A;
	uint8_t calc_val = 0;
	switch (opcode.l) {
	case 0:
		unset_flag(SUBTRACTION_FLAG);
		unset_flag(HALF_CARRY_FLAG);
		switch(opcode.r) {
			case 0:  val = &B; break;
			case 1:  val = &C; break;
			case 2:  val = &D; break;
			case 3:  val = &E; break;
			case 4:  val = &H; break;
			case 5:  val = &L; break;
			default: break;
		}

		calc_val = *val;
		if (opcode.r == 6) {
			tcyc_increment = 16;
			calc_val = mmu.fetch(getHL());
		}

		switch (opcode.m) {
			case 0:  calc_val =  rlc(calc_val); break;
			case 1:  calc_val =  rrc(calc_val); break;
			case 2:  calc_val =   rl(calc_val); break;
			case 3:  calc_val =   rr(calc_val); break;
			case 4:  calc_val =  sla(calc_val); break;
			case 5:  calc_val =  sra(calc_val); break;
			case 6:  calc_val = swap(calc_val); break;
			case 7:  calc_val =  srl(calc_val); break;
			default: break;
		}

		if (calc_val == 0) {
			set_flag(ZERO_FLAG);
		} else {
			unset_flag(ZERO_FLAG);
		}

		if (opcode.r == 6) {
			tcyc_increment = 16;
			mmu.set(getHL(), calc_val);
		} else {
			*val = calc_val;
		}
		break;
	case 1: // BIT b,r BIT b,[HL]
		switch(opcode.r) {
			case 0:  val_noref = B; break;
			case 1:  val_noref = C; break;
			case 2:  val_noref = D; break;
			case 3:  val_noref = E; break;
			case 4:  val_noref = H; break;
			case 5:  val_noref = L; break;
			case 6:  val_noref = mmu.fetch(getHL()); tcyc_increment = 12; break;
			default: break;
		}
		unset_flag(SUBTRACTION_FLAG);
		set_flag(HALF_CARRY_FLAG);
		if ( ( (val_noref >> (int)(opcode.m) ) & 0b1 ) == 0) {
			set_flag(ZERO_FLAG);
		} else {
			unset_flag(ZERO_FLAG);
		}
		break;
	case 2: // RES b,r RES b,[HL]
	case 3: // SET b,r SET b,[HL]
		
		val = &A;
		switch(opcode.r) {
			case 0:  val = &B; break;
			case 1:  val = &C; break;
			case 2:  val = &D; break;
			case 3:  val = &E; break;
			case 4:  val = &H; break;
			case 5:  val = &L; break;
			default: break;
		}

		calc_val = *val;
		if (opcode.r == 6) {
			tcyc_increment = 16;
			calc_val = mmu.fetch(getHL());
		}

		if (opcode.l == 2) {
			calc_val = calc_val & ~(0b1 << opcode.m);
		} else {
			calc_val = calc_val | (0b1 << opcode.m);
		}

		if (opcode.r == 6) {
			tcyc_increment = 16;
			mmu.set(getHL(), calc_val);
		} else {
			*val = calc_val;
		}

		break;
	default:
		break;
	}

	// WAS PREVIOUSLY INCREMENTING, THIS WILL BE CLEANED LATER
	opcode.sz = sz_increment;
	opcode.tcyc = tcyc_increment;
}
