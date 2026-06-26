#include "APU.h"

// Audio implementation based on Rust implementation found here: https://github.com/NightShade256/Argentum/blob/development/crates/argentum/src/audio.rs#L317

uint32_t wave_duty_amplitude(uint8_t wave_duty_pattern, uint8_t wave_duty_position) {

	switch (wave_duty_pattern) {
	case 0:
		if (wave_duty_position == 7) { return 1; }
		break;
	case 1:
		if (wave_duty_position >= 6) { return 1; }
		break;
	case 2:
		if (wave_duty_position >= 4) { return 1; }
		break;
	case 3:
		if (wave_duty_position <= 5) { return 1; }
		break;
	default:
		break;
	}
	return 0;

}

// Channel1

void Channel1::tick() {
	if (freq_timer <= 0) {
		freq_timer = (2048 - freq) * 4;
		wave_pos++;
	}
	if (wave_pos > 7) {
		wave_pos = 0;
	}
	freq_timer--;
}

void Channel1::length_step() {
	if (length_enabled && length_counter > 0) {
		length_counter -= 1;
		if (length_counter <= 0) {
			channel_on = false;
		}
	}
}

void Channel1::volume_step() {
	if (period != 0) {
		if (period_timer > 0) {
			period_timer--;
		}

		if (period_timer <= 0) {
			period_timer = period;
			if (curr_vol < 0xF && incrementing) {
				curr_vol++;
			}
			if (curr_vol > 0 && !incrementing) {
				curr_vol--;
			}
		}
	}	
}

void Channel1::sweep_step() {
	if (sweep_timer > 0) {
		sweep_timer--;
	}

	if (sweep_timer <= 0) {
		if (sweep_period > 0) {
			sweep_timer = sweep_period;
		} else {
			sweep_timer = 8;
		}

		if (sweep_enabled && sweep_period > 0) {
			new_freq = sweep_frequency();

			if (new_freq <= 2047 && sweep_amount > 0) {
				freq = new_freq;
				shadow_freq = new_freq;

				sweep_frequency();
			}
		}

	}
}

uint16_t Channel1::sweep_frequency() {
	uint16_t sweep_freq = (shadow_freq >> sweep_amount);
	if (sweep_decrementing) {
		sweep_freq = shadow_freq - sweep_freq;
	} else {
		sweep_freq = shadow_freq + sweep_freq;
	}

	if (sweep_freq > 2047) {
		channel_on = false;
	}

	return sweep_freq;
}

float Channel1::amplitude() {
	if (dac && channel_on) {
		return (((float)wave_duty_amplitude(duty_pattern, wave_pos) * (float)curr_vol) / 7.5f) - 1.0f;
	}
	return 0;
}

void Channel1::setNR10(uint8_t value) {
	sweep_amount = value & 0x07;
	sweep_decrementing = ((value & 0x08 ) != 0);
	sweep_period = (value >> 4) & 0b111;
}

void Channel1::setNR11(uint8_t value) {
	length_counter = 64 - (value & 0x3F);
	duty_pattern = (value >> 6) & 3;
}// Update

void Channel1::setNR12(uint8_t value) {
	dac = ( (value & 0xF8) != 0 );
	if (!dac) {
		channel_on = false;
	}
	period = value & 7;
	incrementing = ( (value & 8) != 0 );
	init_vol = value >> 4;

}

void Channel1::setNR13(uint8_t value) {
	freq = (freq & 0x0700) | value;
}

void Channel1::setNR14(uint8_t value) {
	freq = (freq & 0xFF) | ( ( (uint16_t)(value & 7) ) << 8);
	length_enabled = ( (value >> 6) & 0x1 ) != 0;
	if (length_counter == 0) {
		length_counter = 64;
	}

	if ( (value >> 7) != 0 && dac) {
		channel_on = true;
		period_timer = period;
		curr_vol = init_vol;

		shadow_freq = freq;
		if (sweep_period > 0) {
			sweep_timer = sweep_period;
		} else {
			sweep_timer = 8;
		}

		sweep_enabled = (sweep_period > 0 || sweep_amount > 0);

		if (sweep_amount > 0) {
			sweep_frequency();
		}
	}
}

uint8_t Channel1::getNR10() { return (sweep_period << 4) | (sweep_decrementing << 3) | sweep_amount | 0x80; }
uint8_t Channel1::getNR11() { return (duty_pattern << 6) | (0b00111111); }
uint8_t Channel1::getNR12() { return (init_vol << 4) | (incrementing << 3) | period; }
uint8_t Channel1::getNR13() { return 0xFF; }
uint8_t Channel1::getNR14() { return (length_enabled << 6) | 0b10111111; }

Channel1::Channel1() { }

// Channel 2

// I used the blog post and GitHub to get an initial idea of how to implement this channel (first I made)
// in order to get an understanding of the Game Boy's APU implementation
// Rest were made based on this
// https://github.com/NightShade256/Argentum/blob/development/crates/argentum/src/audio.rs
void Channel2::tick() {
	if (freq_timer <= 0) {
		freq_timer = (2048 - freq) * 4;
		wave_pos++;
	}
	if (wave_pos > 7) {
		wave_pos = 0;
	}
	freq_timer--;
}

void Channel2::length_step() {
	if (length_enabled && length_counter > 0) {
		length_counter -= 1;
		if (length_counter <= 0) {
			channel_on = false;
		}
	}
}

void Channel2::volume_step() {
	if (period != 0) {
		if (period_timer > 0) {
			period_timer--;
		}

		if (period_timer <= 0) {
			period_timer = period;
			if (curr_vol < 0xF && incrementing) {
				curr_vol++;
			}
			if (curr_vol > 0 && !incrementing) {
				curr_vol--;
			}
		}
	}	
}

float Channel2::amplitude() {
	if (dac && channel_on) {
		return (((float)wave_duty_amplitude(duty_pattern, wave_pos) * (float)curr_vol) / 7.5f) - 1.0f;
	}
	return 0;
}

void Channel2::setNR21(uint8_t value) {
	length_counter = 64 - (value & 0x3F);
	duty_pattern = (value >> 6) & 3;
}

void Channel2::setNR22(uint8_t value) {
	dac = ( (value & 0xF8) != 0 );
	if (!dac) {
		channel_on = false;
	}
	period = value & 7;
	incrementing = ( (value & 8) != 0 );
	init_vol = value >> 4;

}

void Channel2::setNR23(uint8_t value) {
	freq = (freq & 0x0700) | value;
}

void Channel2::setNR24(uint8_t value) {
	freq = (freq & 0xFF) | ( ( (uint16_t)(value & 7) ) << 8);
	length_enabled = ( (value >> 6) & 0x1 ) != 0;
	if (length_counter == 0) {
		length_counter = 64;
	}

	if ( (value >> 7) != 0 && dac) {
		channel_on = true;
		period_timer = period;
		curr_vol = init_vol;
	}
}

uint8_t Channel2::getNR21() { return (duty_pattern << 6) | (0b00111111); }
uint8_t Channel2::getNR22() { return (init_vol << 4) | (incrementing << 3) | period; }
uint8_t Channel2::getNR23() { return 0xFF; }
uint8_t Channel2::getNR24() { return (length_enabled << 6) | 0b10111111; }

Channel2::Channel2() { }

// Channel3

void Channel3::setNR30(uint8_t value) {
	dac = ( ((value >> 7) & 0b1) != 0);
	if (!dac) {
		channel_on = false;
	}
}

void Channel3::setNR31(uint8_t value) {
	length_counter = 256 - (uint16_t)(value);
}

void Channel3::setNR32(uint8_t value) {
	output_level = (value >> 5) & 3;
	switch(output_level) {
	case 0: volume_shift = 4; break;
	case 1: volume_shift = 0; break;
	case 2: volume_shift = 1; break;
	case 3: volume_shift = 2; break;
	}
}

void Channel3::setNR33(uint8_t value) {
	freq = (freq & 0x0700) | value;
}

void Channel3::setNR34(uint8_t value) {
	freq = (freq & 0xFF) | ((value & 7) << 8);
	length_enabled = (((value >> 6) & 1) != 0);

	if (length_counter == 0) {
		length_counter = 256;
	}

	if ( (value >> 7) != 0 && dac) {
		channel_on = true;
	}
}
void Channel3::set_wave_ram(uint16_t address, uint8_t value) {
	wave_ram[address - WAVE_PATTERN] = value;
}

void Channel3::tick() {
	if (freq_timer == 0) {
		freq_timer = (2048 - freq) * 2;
		wave_pos = (wave_pos + 1) & 31;
	}
	freq_timer--;
}

float Channel3::amplitude() {
	if (dac) {
		uint32_t sample = wave_ram[wave_pos / 2];
		if ( (wave_pos & 1) != 0) {
			sample = sample >> 4;
		}
		sample = sample & 0xF;

		return ((float)(sample >> volume_shift) / 7.5f) - 1.0f;
	}
	return 0.0f;
}

void Channel3::length_step() {
	if (length_enabled && length_counter > 0) {
		length_counter--;

		if (length_counter == 0) {
			channel_on = false;
		}
	}
}

uint8_t Channel3::getNR30() { return (dac << 7) | 0x7F; }
uint8_t Channel3::getNR31() { return 0xFF; }
uint8_t Channel3::getNR32() { return (output_level << 5) | 0x9F; }
uint8_t Channel3::getNR33() { return 0xFF; }
uint8_t Channel3::getNR34() { return (length_enabled << 6) | 0b10111111; }
uint8_t Channel3::get_wave_ram(uint16_t address) { return wave_ram[address - WAVE_PATTERN]; }

Channel3::Channel3() { }

// Channel4

void Channel4::tick() {
	if (freq_timer == 0) {
		uint16_t divisor_code = nr43 & 7;
		if (divisor_code == 0) {
			freq_timer = 8;
		} else {
			freq_timer = divisor_code << 4;
		}
		freq_timer = freq_timer << (nr43 >> 4);

		uint16_t xor_val = (lfsr & 1) ^ ((lfsr & 2) >> 1);

		lfsr = (lfsr >> 1) | (xor_val << 14);

		if (((nr43 >> 3) & 1) != 0) {
			lfsr = lfsr & ~(1 << 6);
			lfsr = lfsr | (xor_val << 6);
		}
	}

	freq_timer--;
}

void Channel4::volume_step() {
	if (period != 0) {
		if (period_timer > 0) {
			period_timer--;
		}

		if (period_timer <= 0) {
			period_timer = period;
			if (curr_vol < 0xF && incrementing) {
				curr_vol++;
			}
			if (curr_vol > 0 && !incrementing) {
				curr_vol--;
			}
		}
	}	
}

void Channel4::length_step() {
	if (length_enabled && length_counter > 0) {
		length_counter -= 1;
		if (length_counter <= 0) {
			channel_on = false;
		}
	}
}

float Channel4::amplitude() {
	if (dac && channel_on) {
		return (((float)(~lfsr & 1) * curr_vol) / 7.5f) - 1.0f;
	}
	return 0;
}

void Channel4::setNR41(uint8_t value) {
	length_counter = 64 - (value & 0x3F);
}

void Channel4::setNR42(uint8_t value) {
	dac = ( (value & 0xF8) != 0 );
	if (!dac) {
		channel_on = false;
	}
	period = value & 7;
	incrementing = ( (value & 8) != 0 );
	init_vol = value >> 4;

}

void Channel4::setNR43(uint8_t value) {
	nr43 = value;
}

void Channel4::setNR44(uint8_t value) {
	length_enabled = ( (value >> 6) & 0x1 ) != 0;
	if (length_counter == 0) {
		length_counter = 64;
	}

	if ( (value >> 7) != 0 && dac) {
		channel_on = true;
	}

	if ( (value >> 7) != 0) {
		lfsr = 0x7FFF;
		period_timer = period;
		curr_vol = init_vol;
	}
}

uint8_t Channel4::getNR41() { return 0xFF; }
uint8_t Channel4::getNR42() { return (init_vol << 4) | (incrementing << 3) | period; }
uint8_t Channel4::getNR43() { return nr43; }
uint8_t Channel4::getNR44() { return (length_enabled << 6) | 0b10111111; }

Channel4::Channel4() { }

APU::APU() {
	SDL_Init(SDL_INIT_AUDIO);

	audio.freq = SAMPLE_RATE;
	audio.format = SDL_AUDIO_F32;
	audio.channels = 2;

	obtained.format = SDL_AUDIO_F32;

	stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio, NULL, NULL);

	if (!stream) {
		std::cout << "Failed to load audio stream\n";
	}

	for (int i = 0; i < BUFFER_SIZE; i++) {
		buffer.push_back(0);
	}

	SDL_ResumeAudioStreamDevice(stream);
}

uint8_t APU::io_reg_read(uint16_t address) {
	uint8_t fetched = 0xFF;
	switch (address) {
	case AUD10: fetched = ch1.getNR10(); break;
	case AUD11: fetched = ch1.getNR11(); break;
	case AUD12: fetched = ch1.getNR12(); break;
	case AUD13: fetched = ch1.getNR13(); break;
	case AUD14: fetched = ch1.getNR14(); break;

	case AUD20: fetched = 0xFF; break;
	case AUD21: fetched = ch2.getNR21(); break;
	case AUD22: fetched = ch2.getNR22(); break;
	case AUD23: fetched = ch2.getNR23(); break;
	case AUD24: fetched = ch2.getNR24(); break;

	case AUD30: fetched = ch3.getNR30(); break;
	case AUD31: fetched = ch3.getNR31(); break;
	case AUD32: fetched = ch3.getNR32(); break;
	case AUD33: fetched = ch3.getNR33(); break;
	case AUD34: fetched = ch3.getNR34(); break;

	case AUD40: fetched = 0xFF; break;
	case AUD41: fetched = ch4.getNR41(); break;
	case AUD42: fetched = ch4.getNR42(); break;
	case AUD43: fetched = ch4.getNR43(); break;
	case AUD44: fetched = ch4.getNR44(); break;	

	case AUD50: fetched = (left_on << 7) | (left_vol << 4) | (right_on << 3) | right_vol; break;
	case AUD51: fetched = NR51; break;
	// Update below as more channels are added
	case AUD52: fetched = ( (apu_on << 7) | 0x70 ) | ( (uint8_t)ch4.channel_on << 3) | ( (uint8_t)ch3.channel_on << 2) | ( (uint8_t)ch2.channel_on << 1) | ((uint8_t)ch1.channel_on); break;

	default:
		if (address >= WAVE_PATTERN && address < LCD) {
			ch3.get_wave_ram(address);
		}
		break;
	}
	return fetched;
}

void APU::io_reg_write(uint16_t address, uint8_t value) {
	if (!(apu_on || address == AUD52 || address == AUD11 || address == AUD21 || address == AUD31 || address == AUD41)) {
		return;
	}

	if (!apu_on && (address == AUD11 || address == AUD21 || address == AUD41)) {
		value = value & 0x3F;
	}

	switch(address) {
	// Channel2
	case AUD50:
		left_vol = (value >> 4) & 0x7;
		right_vol = value & 0x7;
		left_on =  ( (value & 0x80) != 0 );
		right_on = ( (value & 0x8) != 0 );
		break;
	case AUD51:
		NR51 = value;
		break;
	case AUD52:
		if ( (value >> 7) == 0 && apu_on) {
			for (int i = AUD10; i <= AUD51; i++) {
				io_reg_write(i, 0);
			}
			apu_on = false;
		} else {
			if ( (value >> 7) != 0 && !apu_on) {
				apu_on = true;
				fs_pos = 0;

				// Update here for channel1 and channel3
				ch1.wave_pos = 0;
				ch2.wave_pos = 0;
				ch3.wave_pos = 0;
			}
		}
		break;
	case AUD10: ch1.setNR10(value); break;
	case AUD11: ch1.setNR11(value); break;
	case AUD12: ch1.setNR12(value); break;
	case AUD13: ch1.setNR13(value); break;
	case AUD14: ch1.setNR14(value); break;

	case AUD21: ch2.setNR21(value); break;
	case AUD22: ch2.setNR22(value); break;
	case AUD23: ch2.setNR23(value); break;
	case AUD24: ch2.setNR24(value); break;

	case AUD30: ch3.setNR30(value); break;
	case AUD31: ch3.setNR31(value); break;
	case AUD32: ch3.setNR32(value); break;
	case AUD33: ch3.setNR33(value); break;
	case AUD34: ch3.setNR34(value); break;

	case AUD41: ch4.setNR41(value); break;
	case AUD42: ch4.setNR42(value); break;
	case AUD43: ch4.setNR43(value); break;
	case AUD44: ch4.setNR44(value); break;	

	default:
		if (address >= WAVE_PATTERN && address < LCD) {
			ch3.set_wave_ram(address, value);
		}
		break;
	}
}

APU::~APU() {
	SDL_DestroyAudioStream(stream);
	SDL_Quit();
}

uint32_t APU::get_sample_size() {
	return SDL_GetAudioStreamQueued(stream);
}

void APU::tick(uint16_t t_cycle) {
	// Update as more channels are added!
	ch1.tick();
	ch2.tick();
	ch3.tick();
	ch4.tick();

	if (t_cycle % 8192 == 0) {
		if (fs_pos % 2 == 0) {
			ch1.length_step();
			ch2.length_step();
			ch3.length_step();
			ch4.length_step();
		}
		if ( (fs_pos + 1) % 8 == 0) {
			ch1.volume_step();
			ch2.volume_step();
			ch4.volume_step();
		}
		if ( fs_pos == 2 | fs_pos == 6) {
			ch1.sweep_step();
		}

		fs_pos = (fs_pos + 1) & 7;
	}

	if (buf_pos < BUFFER_SIZE && t_cycle % ((uint32_t)(DMG_CLOCK / SAMPLE_RATE)) == 0) {
		float left_sample = 0;
		if ( (NR51 & 0x10) != 0) {
			left_sample += ch1.amplitude();
		}
		if ( (NR51 & 0x20) != 0) {
			left_sample += ch2.amplitude();
		}
		if ( (NR51 & 0x40) != 0) {
			left_sample += ch3.amplitude();
		}
		if ( (NR51 & 0x80) != 0) {
			left_sample += ch4.amplitude();
		}
		left_sample = (left_sample / 4.0);
		buffer[buf_pos] = left_sample;

		float right_sample = 0;
		if ( (NR51 & 0x01) != 0) {
			right_sample += ch1.amplitude();
		}
		if ( (NR51 & 0x02) != 0) {
			right_sample += ch2.amplitude();
		}
		if ( (NR51 & 0x04) != 0) {
			right_sample += ch3.amplitude();
		}
		if ( (NR51 & 0x08) != 0) {
			right_sample += ch4.amplitude();
		}
		right_sample = (right_sample / 4.0);
		buffer[buf_pos + 1] = right_sample;

		buf_pos += 2;
	}


	if (buf_pos >= BUFFER_SIZE) {
		buf_pos = 0;
		SDL_PutAudioStreamData(stream, buffer.data(), buffer.size() * sizeof(float));
	}
}

void APU::set_hardware_mode(int mode) {
	hardware_mode = mode;
}
