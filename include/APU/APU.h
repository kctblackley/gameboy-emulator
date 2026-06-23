#include <SDL2/SDL.h>
#include <cstdint>
#include <iostream>
#include <vector>
#include <array>
#include "utility.h"

#define AUD10  0xFF10
#define AUD11  0xFF11
#define AUD12  0xFF12
#define AUD13  0xFF13
#define AUD14  0xFF14
#define AUD20  0xFF15 // not used
#define AUD21  0xFF16
#define AUD22  0xFF17
#define AUD23  0xFF18
#define AUD24  0xFF19
#define AUD30  0xFF1A
#define AUD31  0xFF1B
#define AUD32  0xFF1C
#define AUD33  0xFF1D
#define AUD34  0xFF1E
#define AUD40  0xFF1F // not used
#define AUD41  0xFF20
#define AUD42  0xFF21
#define AUD43  0xFF22
#define AUD44  0xFF23
#define AUD50  0xFF24
#define AUD51  0xFF25
#define AUD52  0xFF26

#define DMG_CLOCK 4194304
#define SAMPLE_RATE 48000
#define BUFFER_SIZE 1024
#define AUDIO_SYNC_FRAME_TOLERANCE 10

class Channel1 {
public:
	uint8_t duty_pattern = 0;
	uint8_t length_counter = 0;
	uint8_t sweep_amount = 0;
	uint8_t sweep_period = 0;
	uint32_t sweep_timer = 0;

	bool sweep_decrementing = false;
	bool incrementing = false;
	bool dac = false;
	bool length_enabled = false;
	bool sweep_enabled = false;
	bool channel_on = false;

	uint8_t init_vol = 0;
	uint8_t curr_vol = 0;
	uint32_t period_timer = 0;
	uint8_t period = 0;

	uint16_t freq = 0;
	uint16_t shadow_freq = 0;
	uint16_t new_freq = 0;
	uint32_t freq_timer = 0;
	uint8_t wave_pos = 0;

	Channel1();

	void tick();
	void length_step();
	void volume_step();
	void sweep_step();
	uint16_t sweep_frequency();

	void setNR10(uint8_t value);
	void setNR11(uint8_t value);
	void setNR12(uint8_t value);
	void setNR13(uint8_t value);
	void setNR14(uint8_t value);

	uint8_t getNR10();
	uint8_t getNR11();
	uint8_t getNR12();
	uint8_t getNR13();
	uint8_t getNR14();

	float amplitude();
};

class Channel2 {
public:
	uint8_t duty_pattern = 0;
	uint8_t length_counter = 0;

	bool incrementing = false;
	bool dac = false;
	bool length_enabled = false;
	bool channel_on = false;

	uint8_t init_vol = 0;
	uint8_t curr_vol = 0;
	uint32_t period_timer = 0;
	uint8_t period = 0;

	uint16_t freq = 0;
	uint32_t freq_timer = 0;
	uint8_t wave_pos = 0;

	Channel2();

	void tick();
	void length_step();
	void volume_step();

	void setNR21(uint8_t value);
	void setNR22(uint8_t value);
	void setNR23(uint8_t value);
	void setNR24(uint8_t value);

	uint8_t getNR21();
	uint8_t getNR22();
	uint8_t getNR23();
	uint8_t getNR24();

	float amplitude();
};

#define WAVE_RAM_SIZE 16

class Channel3 {
public:
	std::array<uint8_t, WAVE_RAM_SIZE> wave_ram = {};

	uint16_t length_counter = 0;
	uint8_t output_level = 0;
	uint8_t volume_shift = 0;

	uint16_t freq = 0;
	uint32_t freq_timer = 0;
	uint8_t wave_pos = 0;

	bool dac = false;
	bool length_enabled = false;
	bool channel_on = false;

	Channel3();

	void tick();
	void length_step();

	void setNR30(uint8_t value);
	void setNR31(uint8_t value);
	void setNR32(uint8_t value);
	void setNR33(uint8_t value);
	void setNR34(uint8_t value);
	void set_wave_ram(uint16_t address, uint8_t value);

	uint8_t getNR30();
	uint8_t getNR31();
	uint8_t getNR32();
	uint8_t getNR33();
	uint8_t getNR34();
	uint8_t get_wave_ram(uint16_t address);

	float amplitude();
};

class Channel4 {
public:
	uint8_t length_counter = 0;

	bool incrementing = false;
	bool dac = false;
	bool length_enabled = false;
	bool channel_on = false;

	uint8_t init_vol = 0;
	uint8_t curr_vol = 0;
	uint16_t lfsr = 0;
	uint32_t period_timer = 0;
	uint32_t freq_timer = 0;
	uint8_t period = 0;
	uint8_t nr43 = 0;

	Channel4();

	void tick();
	void length_step();
	void volume_step();

	void setNR41(uint8_t value);
	void setNR42(uint8_t value);
	void setNR43(uint8_t value);
	void setNR44(uint8_t value);

	uint8_t getNR41();
	uint8_t getNR42();
	uint8_t getNR43();
	uint8_t getNR44();

	float amplitude();
};

class APU {
public:
	bool pause_emulation = false;
	bool can_queue = false;

	Channel1 ch1;
	Channel2 ch2;
	Channel3 ch3;
	Channel4 ch4;

	SDL_AudioSpec audio = {};
	SDL_AudioSpec obtained = {};
	SDL_AudioDeviceID device = {};

	uint8_t fs_pos = 0;
	uint16_t buf_pos = 0;

	bool apu_on = false;
	bool left_on, right_on = false;
	uint8_t left_vol, right_vol = 0;
	uint8_t NR51 = 0;

	std::vector<float> buffer = {};

	APU();
	~APU();
	uint8_t io_reg_read(uint16_t address);
	uint32_t get_sample_size();
	void io_reg_write(uint16_t address, uint8_t value);
	void tick(uint16_t t_cycle);
};