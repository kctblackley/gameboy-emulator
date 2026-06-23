#include "CPU.h"

// Completed and tested

void CPU::execute_1_ops(Opcode& opcode) {
	if (opcode.r == 6) {
		if (opcode.m == 6) {
			is_halt = true;
			opcode.sz = 1; opcode.tcyc = 4;
		} else {
			// LD r8,[HL] (tested)
			uint8_t r8 = mmu.fetch(getHL());
			opcode.tcyc = 8;
			switch(opcode.m) {
				case 0:  B = r8; break;
				case 1:  C = r8; break;
				case 2:  D = r8; break;
				case 3:  E = r8; break;
				case 4:  H = r8; break;
				case 5:  L = r8; break;
				default: A = r8; break;
			}
		}
	} else {
		uint8_t r8 = A; // determined by opcode.r
		switch(opcode.r) {
			case 0:  r8 = B; break;
			case 1:  r8 = C; break;
			case 2:  r8 = D; break;
			case 3:  r8 = E; break;
			case 4:  r8 = H; break;
			case 5:  r8 = L; break;
			default: r8 = A; break;
		}

		if (opcode.m == 6) {
			// LD [HL],r8 (tested)
			opcode.tcyc = 8;
			mmu.set(getHL(), r8);

		} else {
			// LD r8,r8 (tested)
			switch(opcode.m) {
				case 0:  B = r8; break;
				case 1:  C = r8; break;
				case 2:  D = r8; break; // DEBUG MODE NOT ADDED
				case 3:  E = r8; break;
				case 4:  H = r8; break;
				case 5:  L = r8; break;
				default: A = r8; break;
			}

		}

	}
}