#include <iostream>
#include <cstdint>
#include <array>
#include <queue>
#include <deque>
#include "utility.h"

#define BG_PALETTE 0
#define OBJ_PALETTE_0 1
#define OBJ_PALETTE_1 2

typedef struct {
	uint8_t colour; // colour id for that pixel
	int palette; // source + palette
	uint8_t priority_bit; // important for pixel mixing 
} pixel;

typedef struct {
	uint8_t y;
	uint8_t x;
	uint8_t tile_idx;
	uint8_t attr;
	bool top_tile;
} oam_entry;

class PPU {
public:
	std::array<uint8_t, VRAM_SIZE> vram = {};
	std::array<uint8_t, OAM_SIZE> oam = {};
	std::array<uint8_t, IO_REGISTERS_SIZE> io_registers = {};

	std::vector<oam_entry> fetched_oam;

	bool close_window;
	bool request_vblank = false;
	bool request_stat = false;
	bool clear_fifo = false;

	std::array<uint32_t, GAMEBOY_SCREEN_WIDTH * GAMEBOY_SCREEN_HEIGHT> framebuffer = {};

	std::queue<pixel> bg_fifo;
	std::deque<pixel> sprite_fifo;

	pixel transparent_pixel = {0, OBJ_PALETTE_0, 1};

	int mode = 0;
	int ppu_cycle = 0;
	int tile_x = 0;
	uint8_t joypad_bit_5 = 0xFF;
	uint8_t joypad_bit_4 = 0xFF;
	uint8_t pf_step;
	uint8_t sprite_height = 0;
	int pf_cycles;
	bool bg_pixels;
	bool first_push_of_scanline;
	bool rendered_window_pixels;
	bool y_condition = false;
	uint8_t val_wx, val_wy;

	bool old_stat_or = false;
	bool stat_or = false;
	uint8_t stat = 0xFF;
	int8_t spurious_stat_counter = 4;
	bool spurious_stat = false;

	uint8_t val_lx, render_x;
	uint8_t val_ly = 0;
	uint8_t old_ly = 0;
	uint8_t val_lcdc, val_stat;
	uint8_t val_lyc;
	uint8_t val_scx, val_scy;
	uint8_t discard;
	uint8_t tile_id;
	uint16_t tile_number;
	uint16_t tile_c;
	uint8_t window_line_counter = 0;
	uint8_t window_px_counter = 0;
	uint16_t tile_address;

	uint8_t tile_low, tile_high;
	uint8_t lcdc_bit_0, lcdc_bit_1, lcdc_bit_2;

	bool h_blank;
	bool stat_interrupt;

	int hardware_mode = DMG_MODE;

	PPU();
	void set_hardware_mode(int mode);
	uint8_t vram_read(uint16_t address, bool from_cpu = true);
	uint8_t oam_read(uint16_t address, bool from_cpu = true);
	uint8_t io_reg_read(uint16_t address);
	std::array<uint8_t, AUDIO_REG_SIZE> receive_audio();
	void detect_stat();
	void set_stat_mode(int mode);
	void vram_write(uint16_t address, uint8_t value, bool from_cpu = true);
	void oam_write(uint16_t address, uint8_t value, bool from_cpu = true);
	void io_reg_write(uint16_t address, uint8_t value);
	void pixel_fetcher();
	void reset_fetcher();
	void fetch_sprite(oam_entry o);

	uint16_t get_mapped_address(uint8_t address, int address_mode);

	void set_pixel(int x, int y, pixel px, bool blank = false);
	void tick(uint16_t t_cycle);
};