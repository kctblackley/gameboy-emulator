#include "PPU.h"
#include <bitset>
#include <cassert>

#define ADDRESSING_8000 1
#define ADDRESSING_8800 0

PPU::PPU() {
	io_reg_write(LY, 0x00);
    io_reg_write(SCY, 0x00);
    io_reg_write(SCX, 0x00);
    io_reg_write(LCDC, 0x91);
    //io_reg_write(STAT, 0x85);
    ppu_cycle = 0;

    for (int i = 0; i < 8; i++) {
    	sprite_fifo.push_back(transparent_pixel);
    }
}

void PPU::set_pixel(int x, int y, pixel px, bool blank) {
	if (x < 0 || x >= GAMEBOY_SCREEN_WIDTH) {
		return;
	}
	if (y < 0 || y >= GAMEBOY_SCREEN_HEIGHT) {
		return;
	}
	uint8_t palette;
	if (px.palette == BG_PALETTE) {
		palette = io_reg_read(BGP);
	}
	if (px.palette == OBJ_PALETTE_0) {
		palette = io_reg_read(OBP0);
	}
	if (px.palette == OBJ_PALETTE_1) {
		palette = io_reg_read(OBP1);
	}
	
	uint8_t colour_lsb = (palette >> (2 * px.colour)) & 0b1;
	uint8_t colour_msb = (palette >> ((2 * px.colour) + 1)) & 0b1;
	uint8_t colour_bit = (colour_msb << 1) + colour_lsb;

	uint32_t colour = 0xFFFFFFFF;
	switch(colour_bit) {
	case 0:  colour = 0xFFFFFFFF; break;
	case 1:  colour = 0xA9A9A9FF; break;
	case 2:  colour = 0x545454FF; break;
	case 3:  colour = 0x000000FF; break;
	default: break;
	}
	if (blank) {
		colour = 0xFFFFFFFF;
	}

	framebuffer[( y * GAMEBOY_SCREEN_WIDTH) + x] = colour;
}
	
uint8_t PPU::vram_read(uint16_t address, bool from_cpu) {
	//if (mode == 3 && from_cpu)
		//return 0xFF;
	if (hardware_mode == CGB_MODE) {

	} else {
		return vram[address - VRAM];
	}
}

uint8_t PPU::oam_read(uint16_t address, bool from_cpu) {
	//if ( (mode == 2 || mode == 3) && from_cpu)
	//	return 0xFF;
	return oam[address - OAM];
}

uint8_t PPU::io_reg_read(uint16_t address) {
	if (address == STAT) {
		return (io_registers[address - IO_REGISTERS]) | 0b10000000;
	}
	return io_registers[address - IO_REGISTERS];
}

void PPU::vram_write(uint16_t address, uint8_t value, bool from_cpu) {
	//if (mode != 3 || !from_cpu)
	if (hardware_mode == CGB_MODE) {

	} else {
		vram[address - VRAM] = value;
	}
}

void PPU::oam_write(uint16_t address, uint8_t value, bool from_cpu) {
	//if ( (mode != 2 && mode != 3) || !from_cpu)
		oam[address - OAM] = value;
}

void PPU::io_reg_write(uint16_t address, uint8_t value) {
	if (address == JOYPAD) {
		joypad_bit_4 = (value >> 4) & 0b1;
		joypad_bit_5 = (value >> 5) & 0b1;
	}
	if (address == STAT) {
		stat = io_registers[address - IO_REGISTERS];
		stat = (value & 0b11111000) | (stat & 0b111);
		io_registers[address - IO_REGISTERS] = stat;
	} else {
		io_registers[address - IO_REGISTERS] = value;
	}
}

void PPU::set_hardware_mode(int mode) {
	std::cout << "PPU set to mode " << mode << "\n";
	hardware_mode = mode;
}

uint16_t PPU::get_mapped_address(uint8_t address, int address_mode) {
	uint16_t mapped_address = 0x0;
	if (address_mode == ADDRESSING_8000) {
		mapped_address = (uint16_t)(0x8000 + (16 * address));
	}
	if (address_mode == ADDRESSING_8800) {
		mapped_address = (uint16_t)(0x9000 + (int16_t)(16 * (int8_t)(address)));
	}

	return mapped_address;
}

// Just BG fetcher for now
// https://github.com/ISSOtm/pandocs/blob/rendering-internals/src/Rendering_Internals.md#Get%20tile%20ID
void PPU::pixel_fetcher() {
	uint16_t bit_3 = (val_lcdc >> 3) & 0b1;
	uint16_t bit_4 = (val_lcdc >> 4) & 0b1;
	uint16_t bit_5 = (val_lcdc >> 5) & 0b1;
	uint16_t bit_6 = (val_lcdc >> 6) & 0b1;
	uint8_t tile_id_x, tile_id_y;
	uint16_t tile_id_address;

	uint8_t window_shift = 0;
	if (val_wx == 0) {
		window_shift = val_scx % 8;
	}

	if (pf_cycles == 0) {
		switch (pf_step) {

		case 0: // Get tile ID
			// For background only, when windows are added, change accordingly
			if (bg_pixels) {
				tile_id_x = ((((val_lx - 8) + val_scx) & 0xFF) >> 3) & 0x1F;
				tile_id_y = (((val_ly + val_scy) & 0xFF) >> 3) & 0x1F;
				tile_id_address = 0x9800 | (bit_3 << 10) | (tile_id_y << 5) | tile_id_x; 
			} else {
				tile_id_x = (((val_lx) & 0xFF) >> 3) & 0x1F;
				tile_id_y = ((window_line_counter & 0xFF) >> 3) & 0x1F;
				tile_id_address = 0x9800 | (bit_6 << 10) | (tile_id_y << 5) | tile_id_x; 
			}
			tile_id = vram_read(tile_id_address, false);

			pf_cycles = 2;
			pf_step = 1;

			break;

		case 1: // Get tile row (low)
			/*
			if (bit_4 == 1) {
				bit_12 = 0;
			} else {
				bit_12 = ~( (tile_id >> 7) & 0b1);
			}
			tile_address = (0b100 << 13) | (bit_12 << 12) | (tile_id << 4) | (( (val_ly + val_scy) % 8) << 1);
			*/
			if (bit_4 == 1) {
				tile_address = 0x8000 + (16 * (uint8_t)(tile_id));
			} else {
				tile_address = 0x9000 + (16 * (int8_t)(tile_id));
			}
			//tile_address = (bit_4 ? 0x8000 : 0x9000) + (16 * (bit_4 ? tile_id : (int8_t)tile_id));
			if (bg_pixels) {
				tile_address = tile_address + (2 * ((val_ly + val_scy) & 7));
       		} else {
       			tile_address = tile_address + (2 * (window_line_counter & 7));
       		}

			tile_low = vram_read(tile_address, false);

			pf_cycles = 2;
			pf_step = 2;

			break;

		case 2: // Get tile row (high)
			
			tile_high = vram_read(tile_address + 1, false);
			pf_cycles = 2;
			pf_step = 3;
			break;

		case 3:
			pf_cycles = 1;
			pf_step = 3;
			if (bg_fifo.empty()) {
				int to_discard = 8;
				for (int i = 7; i >= 0 && !(bit_5 == 1 && bg_pixels && y_condition && val_lx == val_wx + 1 - window_shift); --i) {
					uint8_t colour_high = ((tile_high >> i) & 0b1) << 1;
					uint8_t colour_low = (tile_low >> i) & 0b1;
					bg_fifo.push({(uint8_t)(colour_high | colour_low), BG_PALETTE, 1});
					val_lx += 1;
					to_discard--;
				}
				if (bit_5 == 1 && bg_pixels && y_condition && val_lx == val_wx + 1 - window_shift) {
					bg_pixels = false;
					val_lx = 0;
					pf_cycles = 6;
					rendered_window_pixels = true;
					discard = to_discard % 8;
				}
				//std::cout << bg_fifo.size( ) << "\n";
				pf_step = 0;
			}

			break;
		}
	}

	pf_cycles -= 1;
	if (pf_cycles <= 0) {
		pf_cycles = 0;
	}
}

void PPU::fetch_sprite(oam_entry o) {
	uint16_t tile_address;
	uint8_t idx = o.tile_idx;

	uint8_t y_flip = (o.attr >> 6) & 0b1;

	if (lcdc_bit_2) {
		if (!y_flip) {
			if (o.top_tile) {
				idx = idx & 0xFE;
			} else {
				idx = idx | 0x01; 
			}
		} else {
			if (o.top_tile) {
				idx = idx | 0x01;
			} else {
				idx = idx & 0xFE; 
			}
		}
	}

	uint8_t palette_bit = (o.attr >> 4) & 0b1;
	uint8_t x_flip = (o.attr >> 5) & 0b1;
	uint8_t priority_bit = (o.attr >> 7) & 0b1;
	
	uint8_t obj_top = o.y - 16;
	uint8_t obj_row = val_ly - obj_top;
	tile_address = 0x8000 + (16 * idx);
	if (!y_flip) {
		tile_address += 2 * (obj_row % 8);
	} else {
		tile_address += 2 * (7 - (obj_row % 8));
	}
	
	uint8_t tile_low = vram_read(tile_address, false);
	uint8_t tile_high = vram_read(tile_address + 1, true);

	uint8_t palette = palette_bit ? OBJ_PALETTE_1 : OBJ_PALETTE_0;

	pixel px;

	int bit = 7;
	if (x_flip) {
		bit = 0;
	}

	for (int i = 0; i <= 7; i++) {
		uint8_t colour_high = ((tile_high >> bit) & 0b1) << 1;
		uint8_t colour_low = (tile_low >> bit) & 0b1;
		px = {(uint8_t)(colour_high | colour_low), palette, priority_bit};
		if (sprite_fifo[i].colour == 0) {
			sprite_fifo[i] = px;
		}
		if (x_flip) {
			bit++;
		} else {
			--bit;
		}
	}
}

void PPU::reset_fetcher() {
	val_lx = 0;
	pf_step = 0;
	pf_cycles = 0;
}

void PPU::set_stat_mode(int mode) {
	uint8_t stat = io_registers[STAT - IO_REGISTERS];
	switch(mode) {
	case 0: stat = (stat & 0b11111100) | 0b00; break;
	case 1: stat = (stat & 0b11111100) | 0b01; break;
	case 2: stat = (stat & 0b11111100) | 0b10; break;
	case 3: stat = (stat & 0b11111100) | 0b11; break;
	}
	io_registers[STAT - IO_REGISTERS] = stat;
}

void PPU::detect_stat() {
	uint8_t stat = io_registers[STAT - IO_REGISTERS];
	uint8_t stat_6 = (stat >> 6) & 0b1;
	uint8_t stat_5 = (stat >> 5) & 0b1;
	uint8_t stat_4 = (stat >> 4) & 0b1;
	uint8_t stat_3 = (stat >> 3) & 0b1;
	bool lyc_lc = (val_ly == val_lyc) && (stat_6 == 1);
	bool mode_2 = (mode == 2) && (stat_5 == 1);
	bool mode_1 = (mode == 1) && (stat_4 == 1);
	bool mode_0 = (mode == 0) && (stat_3 == 1);

	stat_or = (lyc_lc || mode_2 || mode_1 || mode_0);
	if (stat_or && !old_stat_or) {
		request_stat = true;
	}
	old_stat_or = stat_or;
}

void PPU::tick(uint16_t t_cycle) {
	
	val_lyc = io_reg_read(LYC);
	
	old_ly = val_ly;
	val_scx = io_reg_read(SCX);
	val_scy = io_reg_read(SCY);
	val_lcdc = io_reg_read(LCDC);

	lcdc_bit_2 = (val_lcdc >> 2) & 0b1;
	lcdc_bit_1 = (val_lcdc >> 1) & 0b1;
	lcdc_bit_0 = val_lcdc & 0b1;

	val_wx = io_reg_read(WX);
	val_wy = io_reg_read(WY);

	int prev_mode = mode;

	if (val_ly < 144) {
		if (ppu_cycle == 0) {
			rendered_window_pixels = false;
			fetched_oam.clear();
			if (val_ly == val_wy && !y_condition) {
				y_condition = true;
			}
			reset_fetcher();
			mode = 2;
			render_x = 0;
			discard = val_scx & 7;
			bg_pixels = true; // TO CHANGE WHEN ADDING WINDOWS
			first_push_of_scanline = true;
			while(!bg_fifo.empty()) {
				bg_fifo.pop();
			}
		}
		if (ppu_cycle == 80 && lcdc_bit_1 == 1) {
			sprite_height = lcdc_bit_2 ? 16 : 8;
			for (int i = 0; i < 40 && fetched_oam.size() < 10; i++) {
				int oam_idx = ppu_cycle / 2;
				oam_entry obj;
				obj.y = oam_read(OAM + (4 * i));
				obj.x = oam_read(OAM + (4 * i) + 1);
				obj.tile_idx = oam_read(OAM + (4 * i) + 2);
				obj.attr = oam_read(OAM + (4 * i) + 3);
				obj.top_tile = false;
				if (obj.x >= 0 && val_ly + 16 >= obj.y && val_ly + 16 < obj.y + sprite_height) {
					if (sprite_height == 16 && val_ly + 16 < obj.y + 8) {
						obj.top_tile = true;
					}
					fetched_oam.push_back(obj);
				}
			}
		}
		if (ppu_cycle >= 80 && render_x < GAMEBOY_SCREEN_WIDTH + 8) {
			if (bg_fifo.empty()) {
				pixel_fetcher();
			}
			if (!fetched_oam.empty()) {
				for (auto o : fetched_oam) {
					if (o.x == render_x) {
						fetch_sprite(o);
					}
				}
			}
			if (!bg_fifo.empty()) {
				
				bool mixing = false;
				pixel bg_px = bg_fifo.front();
				bg_fifo.pop();
				pixel sprite_px;
				sprite_px = sprite_fifo.front();
				sprite_fifo.pop_front();
				sprite_fifo.push_back(transparent_pixel);
				mixing = true;
				if (lcdc_bit_0 == 0) {
					bg_px.colour = 0;
				}
				if (discard > 0) {
					//std::cout << "DISCARDING\n";
					discard--;
					val_lx--;
				} else {
					pixel final_px = bg_px;
					if (sprite_px.colour != 0 && !(sprite_px.priority_bit == 1 && bg_px.colour != 0)) {
						final_px = sprite_px;
					}
					if (render_x >= 8) {
						set_pixel(render_x - 8, val_ly, final_px, false);
					}
					render_x += 1;
				}
			}
			mode = 3;
			if (render_x >= GAMEBOY_SCREEN_WIDTH) {
				mode = 0;
			}
		}
		if (ppu_cycle >= 80 && render_x >= GAMEBOY_SCREEN_WIDTH + 8) {
			mode = 0;
			val_lx = 0;
		}
	} else {
		y_condition = false;
		mode = 1;
	}
	set_stat_mode(mode);

	if (ppu_cycle == 4 && val_ly == 153) {
		io_reg_write(LY, 0);
	}

	ppu_cycle += 1;

	if (ppu_cycle == 456) {
		ppu_cycle = 0;
		if (val_ly >= 153) {

			window_line_counter = 0;
			y_condition = false;
			val_ly = 0;
		} else {
			val_ly += 1;
		}
		if (y_condition && rendered_window_pixels) {
			window_line_counter += 1;
		}
		if (val_ly == 144 && !request_vblank) {
			request_vblank = true;
		}
	}

	detect_stat();

	if (old_ly != val_ly) {
		io_reg_write(LY, val_ly);
	}
}