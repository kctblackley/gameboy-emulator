#include "CPU.h"

CPU::CPU() { }

void CPU::bootrom() {
	//A = 0x01;
	//B = 0x00;
	//C = 0x13;
	//D = 0x00;
	//E = 0xD8;
	//H = 0x01;
	//L = 0x4D;
	//F = 0xB0;
	//SP = 0xFFFE;
	PC = 0x0000;
	IME = 0;
	set_IME = 0;
}

void CPU::set_hardware_mode(int mode) {
	hardware_mode = mode;
	std::cout << "CPU set to mode " << mode << "\n";
	mmu.set_hardware_mode(mode);
}

void CPU::load_rom(std::string& rom_directory) {
	bootrom();
	mmu.load_rom(rom_directory);
	if (SHOW_CARTRIDGE_CONTENTS) { mmu.print_cartridge(); }
}

void CPU::get_pc_mem() {
	log_PC0 = mmu.fetch(PC, false);
	log_PC1 = mmu.fetch(PC + 1, false);
	log_PC2 = mmu.fetch(PC + 2, false);
	log_PC3 = mmu.fetch(PC + 3, false); 
}

// fetches single instruction and advances timer
void CPU::fetch() {
	PC0 = mmu.fetch(PC);
}

// Fetches of PC1, PC2 with ticking (PC3 unneeded)
uint8_t CPU::PC1() {
	return mmu.fetch(PC + 1, true);
}

uint8_t CPU::PC2() {
	return mmu.fetch(PC + 2, true);
}

void CPU::serial_output() {
	if (mmu.fetch(SC) == 0x81) {
		std::clog << (unsigned char)(mmu.fetch(SB));
		//mmu.set(SC, 0x00);
	}
}


std::string CPU::get_log() {
	std::string log;
	std::stringstream ss;
	ss <<  "A:"  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(A)
	   << " F:"  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(F)
	   << " B:"  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(B)
	   << " C:"  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(C)
	   << " D:"  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(D)
	   << " E:"  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(E)
	   << " H:"  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(H)
	   << " L:"  << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(L)
	   << " SP:" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(getSP())
	   << " PC:" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(PC)
	   << " PCMEM:"
	   << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(log_PC0) << ","
	   << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(log_PC1) << ","
	   << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(log_PC2) << ","
	   << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(log_PC3) << " "
	   << " IF:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(mmu.fetch(IF, false)) << " "
	   << " TIMA:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(mmu.fetch(TIMA, false)) << " "
	   << " TMA:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(mmu.fetch(TMA, false)) << " "
	   << " t-cycle: " << std::dec << (int)(mmu.t_cycle);


	log += ss.str();

	return log;
}

Opcode CPU::extract_lmr(uint8_t op) {
	Opcode opcode;
	opcode.l    = (op >> 6) & 0b111;
	opcode.m    = (op >> 3) & 0b111;
	opcode.r    = op        & 0b111;
	opcode.sz   = 1; // default, only changed if different to this
	opcode.tcyc = 4; // default, only changed if different to this
	return opcode;
}

Opcode CPU::execute() {
	Opcode opcode = extract_lmr(PC0);
	// Only set IME here
	if (set_IME == 1) {
		IME = 1;
		set_IME = 0;
	} // if DI is then called, IME is set back to 0, allowing case where EI followed by DI does not allow interrupts in between
	// Switch statement, but neater...
	switch(opcode.l) { // Octal representation allows easier patterns for detecting opcodes
	case 0: // Each set of ops (given msb of octal representation) has separate file, for organisation
		execute_0_ops(opcode); // CPU_0_ops.cpp
		break;
	case 1:
		execute_1_ops(opcode); // CPU_1_ops.cpp
		break;
	case 2:
		execute_2_ops(opcode); // CPU_2_ops.cpp
		break;
	case 3:
		execute_3_ops(opcode); // CPU_3_ops.cpp
		break;
	default:
		break;
	}

	return opcode; // Opcode struct contains information about t-cycles and size of instruction
}

void CPU::incrementPC(Opcode& opcode) {
	PC += opcode.sz;
}

uint16_t CPU::getHL() {
	return (uint16_t)((H << 8) + L);
}

void CPU::setHL(uint16_t value) {
	H = (uint8_t)(value >> 8);
	L = (uint8_t)(value & 0xFF);
}

uint16_t CPU::getBC() {
	return (uint16_t)((B << 8) + C);
}

void CPU::setBC(uint16_t value) {
	B = (uint8_t)(value >> 8);
	C = (uint8_t)(value & 0xFF);
}

uint16_t CPU::getDE() {
	return (uint16_t)((D << 8) + E);
}

void CPU::setDE(uint16_t value) {
	D = (uint8_t)(value >> 8);
	E = (uint8_t)(value & 0xFF);
}

uint16_t CPU::getSP() {
	return SP;
}

void CPU::setSP(uint16_t value) {
	SP = value;
}

uint16_t CPU::getAF() {
	return (uint16_t)((A << 8) + F);
}

void CPU::setAF(uint16_t value) {
	A = (uint8_t)(value >> 8);
	F = (uint8_t)(value & 0xF0); // must mask bottom four bits
}


uint8_t CPU::get_flag(uint8_t flag) {
	return (F >> flag) & 1;
}

void CPU::set_flag(uint8_t flag) {
	F = F | (1 << flag);
}

void CPU::unset_flag(uint8_t flag) {
	F = F & ~(1 << flag);
}

uint8_t CPU::two_to_power_of(uint8_t pow) { // Efficient two to power of function
	return 0b1 << pow;
}

int CPU::convert_e8_to_decimal(uint8_t e8) { // Utility function for reading of 8-bit two's complement numbers
	int val = 0;
	int add = 0;
	int count = 0;
	while (e8 != 0) {
		if ( (e8 & 0b1) == 0b1) {
			add = two_to_power_of(count);
			if (count == 7) {
				val -= add;
			} else {
				val += add;
			}
		}
		e8 = e8 >> 1;
		count++;
	}
	return val;
}
