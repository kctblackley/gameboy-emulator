// General container for a Gameboy
// Will be abstracted further to account for hardware differences between DMG and CGB

#include "CPU.h"
#include <cstdio>
#include <algorithm>
#include <chrono>
#include <string>
#include <cctype>

#ifndef GB_H
#define GB_H

// For testing of timing of opcodes (t-cycles)
typedef struct {
	uint8_t opcode;
	uint8_t PC1;
	uint8_t PC2;
	uint16_t expected;
	uint16_t received;
} opcode_test;

class GB {
public:
	CPU cpu;
	bool double_speed = false; // False by default for DMG
	bool is_cgb = false;
	bool interrupted = false;
	bool exit_halt = false;

	std::string log;
	std::ofstream ofs;
	std::string username; // For saves

	// For cycle tests (timing)
	std::vector<opcode_test> opcode_tests;
	std::vector<int> tested_opcodes;
	uint16_t prev_t = 0;

	int count = 1;
	int running = 1;

	Opcode opcode = Opcode(); // stores opcode currently being decoded

	int hardware_mode = DMG_MODE;

	GB(); // rom_directory is name of .gb or .gbc rom file

	void set_hardware_mode(int mode);
	void update_if();
	void fetch();
	void log_cpu();
	void execute();
	void cycle_test();
	void increment_pc();
	void serial_output();
	void interrupt_handler();

	void loop();
	bool check_interrupt(uint8_t interrupts, uint8_t bit);
	void run_rom(std::string& rom_directory, bool load_save, std::string& save_file_dir);
};

#endif