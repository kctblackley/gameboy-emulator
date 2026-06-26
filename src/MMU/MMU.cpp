#include "MMU.h"

MMU::MMU() {
	ram_enable = false;
	rom_bank_number = 0;
	ram_bank_number = 0;
	banking_mode = 0;
}

uint8_t unset_bit(uint8_t byte, uint8_t bit) {
	return byte & ~(0b1 << bit);
}

void MMU::set_joypad(bool up, bool down, bool left, bool right, bool a, bool b, bool start, bool select) {
	this->up = up;
	this->down = down;
	this->left = left;
	this->right = right;
	this->a = a;
	this->b = b;
	this->start = start;
	this->select = select;
}

void MMU::load_save(std::string& username) {
	external_ram.fill(0);

	std::string save_directory = "saves/" + rom_name + "/" + username + ".sav";
	std::ifstream in(save_directory, std::ios::binary);

	if (in) {
		in.read(reinterpret_cast<char*>(external_ram.data()), static_cast<std::streamsize>(external_ram.size()));
	} else {
		allow_save = false;
		std::cout << "Failure to load save. Please try again.\n";
	}
}

void MMU::create_save(std::string& username) {
	if (allow_save) {
		std::string save_directory = "saves/" + rom_name + "/" + username + ".sav";
		
		std::filesystem::create_directories(std::filesystem::path(save_directory).parent_path());

		std::ofstream out(save_directory, std::ios::binary | std::ios::trunc);

		if (out) {
			out.write(reinterpret_cast<const char*>(external_ram.data()), static_cast<std::streamsize>(external_ram.size()));
		} else {
			std::cout << "Failure to save. Sorry. :(\n";
		}
	}
}

void MMU::load_rom(std::string& rom_directory) {
	std::ifstream in;
	in.open(rom_directory, std::ios::binary);
	cartridge = std::vector<uint8_t>{std::istreambuf_iterator<char>(in),
                                     std::istreambuf_iterator<char>()};
   	cartridge_type = cartridge[0x0147];
   	
   	std::cout << "CARTRIDGE TYPE: " << std::hex << (int)(cartridge_type) << "\n";
   	battery_enabled = false;
   	if (cartridge_type == 0) {
   		mbc = 0;
   	}
   	if (cartridge_type >= 1 && cartridge_type <= 3) {
   		mbc = 1;
   		if (cartridge_type == 3) {
   			battery_enabled = true;
   		}
   	}
   	if (cartridge_type >= 5 && cartridge_type <= 6) {
   		mbc = 2;
   		if (cartridge_type == 6) {
   			battery_enabled = true;
   		}
   	}
   	if (cartridge_type >= 0x0F && cartridge_type <= 0x13) {
   		mbc = 3;
   		if (cartridge_type == 0x0F || cartridge_type == 0x10 || cartridge_type == 0x13) {
   			battery_enabled = true;
   		}
   	}
   	if (cartridge_type >= 0x19 && cartridge_type <= 0x1E) {
   		mbc = 5;
   		if (cartridge_type = 0x1B || cartridge_type == 0x1E) {
   			battery_enabled = true;
   		}
   	}
   	if (cartridge_type == 0x20) {
   		mbc = 6;
   	}
   	if (cartridge_type == 0x22) {
   		mbc = 7;
   		battery_enabled = true;
   	}
   	
   	// One ROM bank is 16KB
   	switch (cartridge[0x0148]) {
   	case 0: rom_size_kb = 32;  num_rom_banks = 2;  rom_address_bytes = 1; break;
	case 1: rom_size_kb = 64;  num_rom_banks = 4;  rom_address_bytes = 2; break;
	case 2: rom_size_kb = 128; num_rom_banks = 8;  rom_address_bytes = 3; break;
	case 3: rom_size_kb = 256; num_rom_banks = 16; rom_address_bytes = 4; break;
	case 4: rom_size_kb = 512; num_rom_banks = 32; rom_address_bytes = 5; break;
	default: break;
   	}

   	// One RAM bank is 8KB
   	switch(cartridge[0x0149]) {
   	case 0: ram_size_kb = 0;   num_ram_banks = 0;  ram_address_bytes = 0; break; // FOR MBC1, return open bus data ($FF)
   	case 1: ram_size_kb = 0;   num_ram_banks = 0;  ram_address_bytes = 0; break; // Same here
   	case 2: ram_size_kb = 8;   num_ram_banks = 1;  ram_address_bytes = 0; break;
   	case 3: ram_size_kb = 32;  num_ram_banks = 4;  ram_address_bytes = 2; break;
   	case 4: ram_size_kb = 128; num_ram_banks = 16; ram_address_bytes = 4; break;
   	case 5: ram_size_kb = 64;  num_ram_banks = 8;  ram_address_bytes = 3; break;
   	default: break;
   	}
}

void MMU::request_interrupt(uint8_t interrupt) {
	//std::cout << "INTERRUPT REQUESTED\n";
	if (interrupt == INTERRUPT_SERIAL && (fetch(SC, false) & 0b1) == 0) {
		return;
	}
	uint8_t if_byte = fetch(IF, false);
	if_byte = if_byte | (0b1 << interrupt);
	set(IF, if_byte, false, true);
}

void MMU::reset_if(uint8_t interrupt) {
	uint8_t if_byte = fetch(IF, false);
	if_byte = if_byte & ~(0b1 << interrupt);
	set(IF, if_byte, false, true);
}

bool falling_edge(uint16_t val, uint8_t bit) {
	uint16_t prev = val - 1;

	uint8_t bit_prev = (prev >> bit) & 0b1;
	uint8_t bit_curr =  (val >> bit) & 0b1;

	if (bit_prev == 1 && bit_curr == 0) {
		return true;
	} else {
		return false;
	}
}

void MMU::print_cartridge() {
	std::cout << "\n";
	for (auto c : cartridge) {
		std::cout << std::hex << (int)c << " ";
	}
}

uint8_t get_bit(uint16_t val, uint8_t bit) {
	return (val >> bit) & 0b1;
}

void MMU::update_timers(bool writing_tma, uint8_t old_tma) {
	set(DIV, t_cycle >> 8, false, true);
	uint8_t tac = fetch(TAC, false);
	uint8_t tma = 0;
	if (writing_tma) {
		tma = old_tma;
	} else {
		tma = fetch(TMA, false);
	}
	uint16_t enable_bit = get_bit(tac, 2);
	
	if (get_bit(tac, 2) == 1) {
		uint8_t clock_type = tac & 0b11;
		bool increment_tima = false;
		switch (clock_type) {
		case 0:
			increment_tima = falling_edge(t_cycle, 9);
			break;
		case 1:
			increment_tima = falling_edge(t_cycle, 3);
			break;
		case 2:
			increment_tima = falling_edge(t_cycle, 5);
			break;
		case 3:
			increment_tima = falling_edge(t_cycle, 7);
			break;
		default:
			break;
		}
		if (reset_tima) {
			set(TIMA, tma, false);
			reset_tima = false;
		}
		if (increment_tima) {
			uint8_t tima = fetch(TIMA, false);
			if (tima != 0xFF) {
				set(TIMA, tima + 1, false, true);
				if (fetch(TIMA, false) == 0xFF) {
					tima_overflow = true;
				}
			}
		}
	}

	// DIV - upper 8 bits of 16-bit counter
	// TIMA is based on MMU bits and when their falling edge occurs...
}

void MMU::t_tick(bool writing_tma, uint8_t old_tma) {
	t_cycle += 1;
	if (serial_interrupt_request) {
		cycles_since_serial += 1;
	}
	if (cycles_since_serial > 8) {
		set(SB, 0xFF, false, false);
		request_interrupt(INTERRUPT_SERIAL);
		cycles_since_serial = 0;
		serial_interrupt_request = false;
	}
	apu.tick(t_cycle);
	ppu.tick(t_cycle);
	if (ppu.request_vblank == true) {
		update_frame = true;
		request_interrupt(INTERRUPT_VBLANK);
		ppu.request_vblank = false;
	}
	if (ppu.request_stat == true) {
		request_interrupt(INTERRUPT_LCD);
		ppu.request_stat = false;
	}
	update_timers(writing_tma, old_tma);
}

void MMU::m_tick(bool writing_tma, uint8_t old_tma) {
	for (int i = 0; i < T_CYCLE_PER_M; i++) {
		t_tick(writing_tma, old_tma);
	}
	if (dma_transfer) {
		uint8_t val = fetch((uint16_t)(dma_start + dma_iter), false);
		ppu.oam_write((uint16_t)(0xFE00 + dma_iter), val);
		dma_iter += 1;
		dma_delay -= 1;
		if (dma_delay == 0) {
			dma_transfer = false;
		}
	}
	if (dma_delay == 0) {
		dma_transfer = false;
	}
}

void MMU::mbc_set(uint16_t address, uint8_t value) {
	// MBC0
	if (mbc == 0) {
		if (address < VRAM && !dma_transfer) { // MBC0 so bank 0 and 1 already loaded...
			return;
		}
		if (address >= EXTERNAL_RAM && address < WRAM && !dma_transfer) { // BANKING FOR BOTH
			external_ram[address - EXTERNAL_RAM] = value;
		}
	}

	// MBC1
	if (mbc == 1) {
		uint32_t mbc_address = 0;
		if (address <= 0x1FFF) {
			if (value == 0xA) {
				ram_enable = true;
			} else {
				ram_enable = false;
			}
		}
		if (address >= 0x2000 && address <= 0x3FFF) {
			rom_bank_number = value & 0x1F;
		}
		if (address >= 0x4000 && address <= 0x5FFF) {
			ram_bank_number = value & 0b11;
		}
		if (address >= 0x6000 && address <= 0x7FFF) {
			banking_mode = value & 0b1;
		}
		if (address >= 0xA000 && address <= 0xBFFF && !dma_transfer) {
			if (ram_enable) {
				if (banking_mode == 0) {
					mbc_address = (address & 8191);
				} else {
					mbc_address = ((ram_bank_number & 0b11) << 13) | (address & 8191);
				}
				external_ram[mbc_address] = value;
			}
		}
	}

	// MBC2
	if (mbc == 2) {
		if (address <= 0x3FFF) {
			uint8_t address_bit_8 = (address >> 8) & 0b1;
			if (address_bit_8 == 0) {
				if ( (value & 0xF) == 0xA ) {
					ram_enable = true;
				} else {
					ram_enable = false;
				}
			} else {
				rom_bank_number = value & 0xF;
			}
		}
	}

	// MBC3
	if (mbc == 3) {
		if (address <= 0x1FFF) {
			if (value == 0xA) {
				ram_enable = true; // also enables r/w to RTC registers
			} else {
				ram_enable = false;
			}
		}
		if (address >= 0x2000 && address <= 0x3FFF) {
			rom_bank_number = value;
			if (rom_bank_number == 0) {
				rom_bank_number = 1;
			}
		}
		if (address >= 0x4000 && address <= 0x5FFF) {
			if ( (value & 0xF) <= 0x07) {
				rtc_reg_mapped = false;
				ram_bank_number = value & 0xF;
			} else {
				if ( (value & 0xF) <= 0x0C) {
					rtc_reg_mapped = true;
					rtc_reg_number = value & 0xF;
				} else {
					rtc_reg_mapped = false;
				}
			}
		}
		if (address >= 0xA000 && address <= 0xBFFF && !dma_transfer) {
			uint32_t mbc_address = ((ram_bank_number & 0xF) << 13) | (address & 8191);
			external_ram[mbc_address] = value;
		}
	}

	// MBC5
	if (mbc == 5) {
		if (address <= 0x1FFF) {
			if (value == 0xA) {
				ram_enable = true; // also enables r/w to RTC registers
			} else {
				ram_enable = false;
			}
		}
		if (address >= 0x2000 && address <= 0x2FFF) {
			rom_bank_number = rom_bank_number & (0xFF00);
			rom_bank_number = rom_bank_number | value;
		}
		if (address >= 0x3000 && address <= 0x3FFF) {
			rom_bank_number = rom_bank_number & (0xFEFF);
			rom_bank_number = rom_bank_number | ((value & 1) << 8);
		}
		if (address >= 0x4000 && address <= 0x5FFF) {
			ram_bank_number = value & 0xF;
		}
		if (address >= 0xA000 && address <= 0xBFFF && !dma_transfer) {
			uint32_t mbc_address = ((ram_bank_number & 0xF) << 13) | (address & 8191);
			external_ram[mbc_address] = value;
		}
	}

}

void MMU::set_hardware_mode(int mode) {
	hardware_mode = mode;
	std::cout << "MMU set to mode " << mode << "\n";
	ppu.set_hardware_mode(mode);
	apu.set_hardware_mode(mode);
}

void MMU::set_wram(uint16_t address, uint8_t value) {
	if (hardware_mode == CGB_MODE) {
		if (address < WRAM_SWITCHABLE) {
			wram[address - WRAM] = value;
		} else {
			uint8_t svbk = ppu.io_reg_read(SVBK) & 0b111;
			if (svbk == 0) {
				svbk = 1;
			}
			wram[address - WRAM_SWITCHABLE + (4096 * svbk)] = value;
		}
	} else {
		wram[address - WRAM] = value;
	}
}

void MMU::set(uint16_t address, uint8_t value, bool tick, bool timer_update) {
	bool writing_tma = false;
	if (address == TMA) {
		writing_tma = true;
	}
	mbc_set(address, value);
	if (address >= VRAM && address < EXTERNAL_RAM && !dma_transfer) { // BANKING FOR CGB
		ppu.vram_write(address, value);
	}
	if (address == DIV && !timer_update && !dma_transfer) {
		value = 0x00;
	}
	if (MEM_LOG_ENABLED == 1) { std::cout << std::hex << (int)value << " was written to address " << std::hex << (int)address << std::endl; }
	if (address >= WRAM && address < WRAM_SWITCHABLE && !dma_transfer) {
		set_wram(address, value);
		wram[address - WRAM] = value;
	}
	if (address >= WRAM_SWITCHABLE && address < ECHO_RAM && !dma_transfer) { // BANKING FOR CGB
		set_wram(address, value);
	}
	if (address >= ECHO_RAM && address < OAM && !dma_transfer) {
		set_wram(address - ECHO_RAM + WRAM, value);
	}
	if (address >= OAM && address < NOT_USABLE) {
		if (dma_transfer) {
			return;
		} else {
			ppu.oam_write(address, value);
		}
	}
	if (address >= IO_REGISTERS && address < HRAM) {
		if (address >= 0xFF10 && address < LCD) {
			apu.io_reg_write(address, value);
		} else {
			ppu.io_reg_write(address, value);
		}
	}
	if (address >= HRAM && address < IE_REGISTER) {
		hram[address - HRAM] = value;
	}
	if (address == 0xFFFF) {
		IE = value;
	}
	if (address == 0xFF50) {
		finished_boot = true;
	}
	if (address == 0xFF46) {
		dma_transfer = true;
		dma_delay = 160;
		dma_iter = 0;
		dma_start = value << 8;
	}
	if (address == SC && (((value >> 7) & 0b1) == 1) && prev_sc7 == 0) {
		serial_interrupt_request = true;
		cycles_since_serial = 0;
	}
	uint8_t old_tma = fetch(TMA, false);
	if (tick) { m_tick(writing_tma, old_tma); }
	prev_sc7 = ((value >> 7) & 0b1);
}

bool MMU::is_bootrom(uint16_t address) {
	if (hardware_mode == CGB_MODE) {
		return ( (address < 0x0100 || (address >= 0x0200 && address < 0x0900)) && !finished_boot);
	} else {
		return (address < 0x0100 && !finished_boot);
	}
}

uint8_t MMU::get_bootrom(uint16_t address) {
	if (hardware_mode == CGB_MODE) {
		return bootrom_gbc[address];
	} else {
		return bootrom_gb[address];
	}
}

uint8_t MMU::mbc_fetch(uint16_t address) {
	uint8_t fetched = 0xFF;
	//std::cout << std::hex << (int)interpret_joypad(jp);
	// MBC0
	if (mbc == 0) {
		if (address < VRAM) { // MBC0 so bank 0 and 1 already loaded...
			if (address < 0x0100 && !finished_boot) {
				fetched = bootrom_gb[address];
			} else {
				fetched = cartridge[address];
			}
		}
		if (address >= EXTERNAL_RAM && address < WRAM) { // BANKING FOR BOTH
			fetched = external_ram[address - EXTERNAL_RAM];
		}
	}

	// MBC1
	if (mbc == 1) {
		uint32_t mbc_address = 0;
		if (address <= 0x3FFF) {
			if (is_bootrom(address)) {
				fetched = get_bootrom(address);
			} else {
				if (rom_size_kb >= 1024 && banking_mode == 1) {
					mbc_address = ((ram_bank_number & 3) << 19) | (address & 16383);
				} else {
					mbc_address = (address & 16383);
				}
				fetched = cartridge[mbc_address];
			}
		}
		if (address >= 0x4000 && address <= 0x7FFF) {
			uint8_t masked_rom_bank_reg = rom_bank_number & 31;
			uint8_t selected_rom_bank = 0;
			if (masked_rom_bank_reg == 0) {
				masked_rom_bank_reg = 1;
			}
			if (rom_address_bytes <= 5) {
				selected_rom_bank = (masked_rom_bank_reg % num_rom_banks);
			} else {
				selected_rom_bank = ((ram_bank_number & 0b11) << 5) | (masked_rom_bank_reg % num_rom_banks);
			}

			mbc_address = (selected_rom_bank << 14) | (address & 16383);
			fetched = cartridge[mbc_address];
		}
		if (address >= 0xA000 && address <= 0xBFFF) {
			if (ram_enable) {
				if (banking_mode == 0) {
					mbc_address = (address & 8191);
				} else {
					mbc_address = ((ram_bank_number & 0b11) << 13) | (address & 8191);
				}
				fetched = external_ram[mbc_address];
			} else {
				fetched = 0xFF;
			}
		}
	}

	// MBC2
	if (mbc == 2) {
		if (address <= 0x3FFF) {
			if (is_bootrom(address)) {
				fetched = get_bootrom(address);
			} else {
				fetched = cartridge[address];
			}
		}
		if (address >= 0x4000 && address <= 0x7FFF) {
			uint8_t masked_rom_bank_reg = rom_bank_number & 31;
			uint8_t selected_rom_bank = 0;
			if (masked_rom_bank_reg == 0) {
				masked_rom_bank_reg = 1;
			}
			if (rom_address_bytes <= 5) {
				selected_rom_bank = (masked_rom_bank_reg % num_rom_banks);
			} else {
				selected_rom_bank = ((ram_bank_number & 0b11) << 5) | (masked_rom_bank_reg % num_rom_banks);
			}

			uint32_t mbc_address = (selected_rom_bank << 14) | (address & 16383);
			fetched = cartridge[mbc_address];
		}
		if (address >= 0xA000 && address <= 0xBFFF) {
			if (ram_enable) {
				uint16_t masked_address = address & 0x1FF;
				fetched = external_ram[masked_address] & 0xF;
			} else {
				fetched = 0xFF;
			}
		}
	}

	// MBC3
	if (mbc == 3) {
		if (address <= 0x3FFF) {
			if (is_bootrom(address)) {
				fetched = get_bootrom(address);
			} else {
				fetched = cartridge[address];
			}
		}
		if (address >= 0x4000 && address <= 0x7FFF) {
			uint32_t mbc_address = (rom_bank_number << 14) | (address & 16383);
			fetched = cartridge[mbc_address];
		}
		if (address >= 0xA000 && address <= 0xBFFF) {
			if (rtc_reg_mapped) {

			} else {
				uint32_t mbc_address = (ram_bank_number << 13) | (address & 8191);
				fetched = external_ram[mbc_address];
			}
		}
	}

	// MBC5
	if (mbc == 5) {
		if (address <= 0x3FFF) {
			if (is_bootrom(address)) {
				fetched = get_bootrom(address);
			} else {
				fetched = cartridge[address];
			}
		}
		if (address >= 0x4000 && address <= 0x7FFF) {
			uint32_t mbc_address = ((rom_bank_number & 0x1FF) << 14) | (address & 16383);
			fetched = cartridge[mbc_address];
		}
		if (address >= 0xA000 && address <= 0xBFFF) {
			if (ram_enable) {
				uint32_t mbc_address = ((ram_bank_number & 0b11) << 13) | (address & 8191);
				fetched = external_ram[mbc_address];
			} else {
				fetched = 0xFF;
			}
		}
	}

	return fetched;
}

uint8_t MMU::fetch_wram(uint16_t address) {
	if (hardware_mode == CGB_MODE) {
		if (address < WRAM_SWITCHABLE) {
			return wram[address - WRAM];
		} else {
			uint8_t svbk = ppu.io_reg_read(SVBK) & 0b111;
			if (svbk == 0) {
				svbk = 1;
			}
			return wram[address - WRAM_SWITCHABLE + (4096 * svbk)];
		}
	} else {
		return wram[address - WRAM];
	}
}

uint8_t MMU::fetch(uint16_t address, bool tick) {
	uint8_t fetched = mbc_fetch(address);
	if (address >= VRAM && address < EXTERNAL_RAM) { // BANKING FOR CGB
		fetched = ppu.vram_read(address);
	}
	if (address >= WRAM && address < WRAM_SWITCHABLE) {
		fetched = fetch_wram(address);
	}
	if (address >= WRAM_SWITCHABLE && address < ECHO_RAM) { // BANKING FOR CGB
		fetched = fetch_wram(address);
	}
	if (address >= ECHO_RAM && address < OAM) {
		fetched = fetch_wram(address - ECHO_RAM + WRAM);
	}
	if (address >= OAM && address < NOT_USABLE) {
		if (dma_transfer) {
			fetched = 0xFF;
		} else {
			fetched = ppu.oam_read(address);
		}
	}
	if (address >= NOT_USABLE && address < IO_REGISTERS) {
		fetched = NOT_USABLE_DEFAULT_RETURN;
	}
	if (address >= IO_REGISTERS && address < HRAM) {
		if (address >= 0xFF10 && address < LCD) {
			fetched = apu.io_reg_read(address);
		} else {
			fetched = ppu.io_reg_read(address);
		}
	}
	if (address >= HRAM && address < IE_REGISTER) {
		fetched = hram[address - HRAM];
	}
	if (address == 0xFFFF) {
		fetched = IE;
	}
	if (address == JOYPAD) {
		uint8_t byte = 0xFF;
		if (ppu.joypad_bit_4 == 0) {
			byte = unset_bit(byte, 4);
		}
		if (ppu.joypad_bit_5 == 0) {
			byte = unset_bit(byte, 5);
		}
		if (ppu.joypad_bit_5 == 0) {
			if (start) { byte = unset_bit(byte, 3); }
			if (select) { byte = unset_bit(byte, 2); }
			if (b) { byte = unset_bit(byte, 1); }
			if (a) { byte = unset_bit(byte, 0); }
		} else {
			if (ppu.joypad_bit_4 == 0) {
				if (down) { byte = unset_bit(byte, 3); }
				if (up) { byte = unset_bit(byte, 2); }
				if (left) { byte = unset_bit(byte, 1); }
				if (right) { byte = unset_bit(byte, 0); }
			}
		}
		if (old_joypad_byte != byte) {
			if ((old_joypad_byte & 0xF) < (byte & 0xF)) {
				request_interrupt(INTERRUPT_JOYPAD);
			}
		}
		old_joypad_byte = byte;
		fetched = byte;
		/*std::cout << "FETCHED BUTTONS " << std::hex << (int)fetched;*/
	}
	if (tick) { m_tick(); }
	return fetched;
}