// CPU itself
// Handles execution via lookup using function pointers (coded and defined in operation.h)

#include "MMU.h"

#include <iomanip>
#include <sstream>
#include <string>

#ifndef CPU_H
#define CPU_H

typedef struct { // splits opcode into octal components for easy checking
	uint8_t l; // upper 3-bits
	uint8_t m; // middle 3-bits
	uint8_t r; // lower 3-bits
	uint16_t sz; // to increment pc by after completion of opcode
	uint16_t tcyc; // to set t_cycles too for accurate timing
} Opcode;

class CPU {
public:
	MMU mmu; // MMU handles all read-writes, can be an MBC

	// registers
	uint8_t A, B, C, D, E, H, L;

	// 16-bit registers
	uint16_t SP, PC;

	// PCMEM
	uint8_t PC0; // stores content at PC + 0
	uint8_t log_PC0;
	uint8_t log_PC1; // stores content at PC + 1
	uint8_t log_PC2; // stores content at PC + 2
	uint8_t log_PC3; // stores content at PC + 3
	uint8_t IME;
	uint8_t set_IME = 0; // flag used to delay effect of EI by one instruction
	// flags
	uint8_t F;
	bool jump_flag = false;
	bool is_halt = false;

	int hardware_mode = DMG_MODE;

	CPU(); // Nothing to initialise

	void set_hardware_mode(int mode);

	uint8_t PC1();
	uint8_t PC2();

	void bootrom();
	void load_rom(std::string& rom_directory);
	void fetch();
	void get_pc_mem();

	void serial_output();

	void execute_0_ops(Opcode& opcode);
	void execute_1_ops(Opcode& opcode);
	void execute_2_ops(Opcode& opcode);
	void execute_3_ops(Opcode& opcode);
	void execute_cb_ops(Opcode& opcode);
	Opcode execute();
	Opcode extract_lmr(uint8_t op);

	std::string get_log();

	void set_half_carry_addition(uint8_t a, uint8_t r8);
	void set_half_carry_subtraction(uint8_t a, uint8_t r8);
	void set_carry_addition(uint8_t a, uint8_t r8);
	void set_carry_subtraction(uint8_t a, uint8_t r8);

	void set_16_bit_half_carry_addition(uint16_t a, uint16_t b);
	void set_16_bit_carry_addition(uint16_t a, uint16_t b);
	
	void add_a(uint8_t r8);
	void adc_a(uint8_t r8);
	void sub_a(uint8_t r8);
	void sbc_a(uint8_t r8);
	void and_a(uint8_t r8);
	void xor_a(uint8_t r8);
	void  or_a(uint8_t r8);
	void  cp_a(uint8_t r8);

	void rlca();
	void rrca();
	void rla();
	void rra();
	void daa();
	void cpl();
	void scf();
	void ccf();

	uint8_t  rlc(uint8_t r8);
	uint8_t  rrc(uint8_t r8);
	uint8_t   rl(uint8_t r8);
	uint8_t   rr(uint8_t r8);
	uint8_t  sla(uint8_t r8);
	uint8_t  sra(uint8_t r8);
	uint8_t swap(uint8_t r8);
	uint8_t  srl(uint8_t r8);

	void pushn16(uint16_t n16);
	uint16_t popn16();
	void calla16(uint16_t n16);
	void rst(uint16_t n16);
	void incrementPC(Opcode& opcode);

	uint16_t getHL(); // returns value of HL
	void setHL(uint16_t value);
	uint16_t getBC(); // returns value of BC
	void setBC(uint16_t value);
	uint16_t getDE(); // returns value of DE
	void setDE(uint16_t value);
	uint16_t getSP(); // returns value of SP
	void setSP(uint16_t value);
	uint16_t getAF(); // returns value of AF
	void setAF(uint16_t value);
	

	uint8_t get_flag(uint8_t flag);

	void set_flag(uint8_t flag);
	void unset_flag(uint8_t flag);

	uint8_t two_to_power_of(uint8_t pow);
	int convert_e8_to_decimal(uint8_t e8);

};

#endif