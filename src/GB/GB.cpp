#include "GB.h"

#define PROGRESS_STEP 10000 // Iterations to carry out until asking to quit
#define PROGRESS_STEP_ENABLED 0
#define AUDIO_SAMPLES_PER_FRAME 800
#define MAXIMUM_ITERATIONS 200000
#define LIMIT_EXECUTION 0

GB::GB() {
	// Clear log file
	ofs.open("log.log", std::ofstream::out | std::ofstream::trunc);
	ofs.close();

	// Redirect clog strea to log file
	std::freopen( "log.log", "w", stderr );
}

bool test_comparison(const opcode_test& a, const opcode_test& b) {
	return a.opcode < b.opcode;
}

void GB::fetch() {
	cpu.fetch();
	cpu.jump_flag = false;
}

void GB::log_cpu() {
	if (LOG_ENABLED == 1) {
		cpu.get_pc_mem();
		log = cpu.get_log();
		std::clog << log << std::endl;
	}
}

void GB::execute() {
	opcode = cpu.execute();
}

void GB::cycle_test() {
	if (CYCLE_TEST_ENABLED == 1) {
		uint16_t new_t = cpu.mmu.t_cycle;
		uint16_t diff_t = new_t - prev_t;
		if (diff_t < 0) {
			diff_t += 65536;
		}
		opcode_test test = opcode_test();
		test.opcode = cpu.PC0;
		test.PC1 = cpu.log_PC1;
		test.PC2 = cpu.log_PC2;
		test.expected = opcode.tcyc;
		test.received = diff_t;
		if (cpu.PC0 != 203) {
			if (std::count(tested_opcodes.begin(), tested_opcodes.end(), test.opcode) == 0) {
				opcode_tests.push_back(test);
				tested_opcodes.push_back(test.opcode);
			}
		} else {
			if (std::count(tested_opcodes.begin(), tested_opcodes.end(), test.opcode * test.PC1) == 0) {
				opcode_tests.push_back(test);
				tested_opcodes.push_back(test.opcode * test.PC1);
			}
		}
	}
}

void GB::increment_pc() {
	if (cpu.jump_flag == false) {
		cpu.incrementPC(opcode);
	}
}

void GB::serial_output() {
	if (SERIAL_ENABLED == 1) {
		cpu.serial_output();
	}
}

void GB::update_if() {
	if (cpu.mmu.tima_overflow) {
		cpu.mmu.request_interrupt(INTERRUPT_TIMER);
		cpu.mmu.tima_overflow = false;
		cpu.mmu.reset_tima = true;
	}
}

void GB::run_rom(std::string& rom_directory, bool load_save, std::string& username) {
	
	cpu.load_rom(rom_directory);

	if (cpu.mmu.battery_enabled) {
		if (load_save) {
			this->username = username;
			cpu.mmu.load_save(username);
		} else {
			std::cout << "Starting a new save. Enter a username...\n>> ";
			std::cin >> username;
			username.erase(
				std::remove_if(username.begin(), username.end(),
								[](unsigned char c) {
									return !std::isalpha(c);
								}),
				username.end()
			);
		}
	}

	
	count = 1;
	running = 1;

	opcode_tests.clear();
	tested_opcodes.clear();

	bool processed_interrupt = false;

	interrupted = false;
	exit_halt = false;

	int px_size;
	std::cout << "Which pixel size would you like? (1 = small)\n>> ";
	std::cin >> px_size;
	
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow(
		"Gameboy Emulator",
		GAMEBOY_SCREEN_WIDTH  * px_size,
		GAMEBOY_SCREEN_HEIGHT * px_size,
		0
	);

	SDL_Renderer* renderer = SDL_CreateRenderer(
		window,
		NULL
	);

	SDL_Texture* texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_STREAMING,
		GAMEBOY_SCREEN_WIDTH,
		GAMEBOY_SCREEN_HEIGHT
	);

	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

	SDL_SetRenderLogicalPresentation(renderer, GAMEBOY_SCREEN_WIDTH, GAMEBOY_SCREEN_HEIGHT, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
	
	int frame = 0;
	bool first_frame = true;
	float frame_rate;
	float frame_rate_time = 1000.0 / 59.7;
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	while (running > 0) {
		uint32_t samples_left = cpu.mmu.apu.get_sample_size();
		while (samples_left < sizeof(float) * BUFFER_SIZE * 5) {
			if (!cpu.is_halt) { log_cpu(); }
			update_if();
			interrupt_handler();
			if (!cpu.is_halt) {
				prev_t = cpu.mmu.t_cycle;
				fetch();
				execute();
				increment_pc();
				cycle_test();
			} else {
				cpu.mmu.m_tick();
			}
			if (exit_halt) {
				cpu.is_halt = false;
				exit_halt = false;
			}
			
			//serial_output();
			
			if (cpu.mmu.update_frame) {
				// Keyboard controls and joypad
		        SDL_PumpEvents();
		        int num_keys;
				const bool* state = SDL_GetKeyboardState(&num_keys);

				SDL_Event event;
				if (SDL_PollEvent(&event)) {
					if (event.type == SDL_EVENT_QUIT) {
						running = 0;
					}
				}

				bool up = state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_W];
				bool down = state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S];
				bool left = state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A];
				bool right = state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D];
				bool a = state[SDL_SCANCODE_X] || state[SDL_SCANCODE_K];
				bool b = state[SDL_SCANCODE_Z] || state[SDL_SCANCODE_J];
				bool start = state[SDL_SCANCODE_RETURN] || state[SDL_SCANCODE_RETURN2] || state[SDL_SCANCODE_SPACE];
				bool select = state[SDL_SCANCODE_RSHIFT] || state[SDL_SCANCODE_LSHIFT];
				cpu.mmu.set_joypad(up, down, left, right, a, b, start, select);

				// Render texture
				SDL_UpdateTexture(
					texture,
					nullptr,
					cpu.mmu.ppu.framebuffer.data(),
					GAMEBOY_SCREEN_WIDTH * sizeof(uint32_t)
				);
				SDL_RenderTexture(renderer, texture, nullptr, nullptr);
		        SDL_RenderPresent(renderer);
		        cpu.mmu.update_frame = false;
		        frame++;
		        //std::cout << (int)cpu.mmu.t_cycle << "\n";
		    }
		    samples_left = cpu.mmu.apu.get_sample_size();
		}
	}

	if (cpu.mmu.battery_enabled) {
		cpu.mmu.create_save(username);
	}

	std::sort(opcode_tests.begin(), opcode_tests.end(), test_comparison);
	
	int incorrect_tests = 0;

	/*if (CYCLE_TEST_ENABLED == 1) {
		for (auto t : opcode_tests) {
			if (t.expected != t.received) {
				incorrect_tests += 1;
			}
		}
		if (incorrect_tests == 0) {
			std::cout << "CYCLE TEST PASSED" << "\n";
		} else {
			std::cout << "TEST FAILED\nINCORRECT TIMING:\n\n";

			for (auto t : opcode_tests) {
				if (t.expected != t.received) {
					if (t.opcode != 203) { 
						std::cout << "Opcode: " << std::oct << (int)(t.opcode) << " - Calculated time: " << std::dec << (int)(t.received) << " - Expected time: " << std::dec << (int)(t.expected) << "\n";
					} else {
						std::cout << "CB Opcode: " << "CB" << std::oct << (int)(t.PC1) << " - Calculated time: " << std::dec << (int)(t.received) << " - Expected time: " << std::dec << (int)(t.expected) << "\n";			
					}
				}
			}
		}

		std::cout << "\nCORRECT TIMING:\n\n";
		for (auto t : opcode_tests) {
			if (t.expected == t.received) {
				if (t.opcode != 203) { 
					std::cout << std::oct << (int)(t.opcode) << " ";
				} else {
					std::cout << "CB" << std::oct << (int)(t.PC1) << " ";
				}
			}
		}
		std::cout << "\n\n";
	}*/
}

bool GB::check_interrupt(uint8_t interrupts, uint8_t bit) {
	return ((interrupts >> bit) & 0b1) == 1;
}

void GB::interrupt_handler() {
	uint8_t if_byte = cpu.mmu.fetch(IF, false);
	uint8_t ie_byte = cpu.mmu.fetch(IE_REGISTER, false);
	uint8_t interrupts = if_byte & ie_byte;
	interrupted = false;
	if (interrupts > 0) {
		exit_halt = true;
	}
	if (interrupts > 0 && cpu.IME == 1) {
		uint8_t new_pc = 0x0;
		uint8_t reset_interrupt = 0;
		cpu.IME = 0;
		bool passed = true;
		if (check_interrupt(interrupts, INTERRUPT_JOYPAD)) { new_pc = 0x60; reset_interrupt = INTERRUPT_JOYPAD; }
		if (check_interrupt(interrupts, INTERRUPT_SERIAL)) { new_pc = 0x58; reset_interrupt = INTERRUPT_SERIAL; }
		if (check_interrupt(interrupts, INTERRUPT_TIMER))  { new_pc = 0x50; reset_interrupt = INTERRUPT_TIMER; }
		if (check_interrupt(interrupts, INTERRUPT_LCD))    { new_pc = 0x48; reset_interrupt = INTERRUPT_LCD; }
		if (check_interrupt(interrupts, INTERRUPT_VBLANK)) { new_pc = 0x40; reset_interrupt = INTERRUPT_VBLANK; }
		interrupted = true;
		opcode.sz = 1;
		cpu.mmu.reset_if(reset_interrupt);
		cpu.mmu.m_tick();
		cpu.mmu.m_tick();
		cpu.pushn16(cpu.PC);
		cpu.PC = new_pc;
		cpu.mmu.m_tick();
	}
}