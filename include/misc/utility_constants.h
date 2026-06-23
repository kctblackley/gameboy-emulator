// Contains useful type definitions, constants from Gameboy specification
// Reference: https://gbdev.io/pandocs/Specifications.html
// Reference: https://gbdev.io/pandocs/Memory_Map.html#memory-map
// Reference: https://gbdev.io/pandocs/Hardware_Reg_List.html
// Reference: https://gbdev.io/pandocs/The_Cartridge_Header.html

#include <cstdint>

// General constants (same for DMG and CGB)

// Flag constants
#define ZERO_FLAG        7
#define SUBTRACTION_FLAG 6
#define HALF_CARRY_FLAG  5
#define CARRY_FLAG       4

#define DEFAULT_JOYPAD {false, false, false, false, false, false, false, false}

// in pixels
#define GAMEBOY_SCREEN_WIDTH 160
#define GAMEBOY_SCREEN_HEIGHT 144

// in pixels
#define OBJ_SIZE1_WIDTH 8
#define OBJ_SIZE1_HEIGHT 8
#define OBJ_SIZE2_WIDTH 8
#define OBJ_SIZE2_HEIGHT 16
#define OBJ_MAX_PER_SCREEN 40
#define OBJ_MAX_PER_LINE 10

// system clock frequency ratio
#define SYSTEM_CLOCK_FREQUENCY 0.25 // for both systems, system clock is 1/4 frqeuency of master clock
#define T_CYCLE_PER_M 4

// in hertz
#define HORIZONTAL_SYNC 9.198
#define VERTICAL_SYNC 59.73

// number of channels
#define CHANNELS 4

// Interrupt bits in IF and IE
#define INTERRUPT_VBLANK 0
#define INTERRUPT_LCD 1
#define INTERRUPT_TIMER 2
#define INTERRUPT_SERIAL 3
#define INTERRUPT_JOYPAD 4


// Differing constants (different for DMG and CGB)

// in mega-hertz
#define DMG_MASTER_CLOCK 4.194304
#define CGB_MASTER_CLOCK 8.388608




// Memory map (CGB uses switchable banks)

// addresses are the starts of the areas of memory
#define ROM_BANK00      0x0000 // typically fixed bank for both -> not needed, but here for completeness
#define ROM_BANK01NN    0x4000 // switchable bank via MBC mapper for both
#define VRAM            0x8000 // switchable banks in CGB
#define EXTERNAL_RAM    0xA000 // switchable if any for both
#define WRAM            0xC000 
#define WRAM_SWITCHABLE 0xD000 // switchable bank 1-7 in CGB mode
#define ECHO_RAM        0xE000 // echo RAM mirroriing C000-DDFF, use of area prohibited
#define OAM             0xFE00
#define NOT_USABLE      0xFEA0 // use of area prohibited
#define IO_REGISTERS    0xFF00
#define HRAM            0xFF80
#define IE_REGISTER     0xFFFF

#define NOT_USABLE_DEFAULT_RETURN 0xFF

// RAM sizes
#define VRAM_SIZE             8192
#define EXTERNAL_RAM_SIZE     8192
#define MAX_EXTERNAL_RAM_SIZE 131072
#define WRAM_SIZE             8192
#define OAM_SIZE              160
#define IO_REGISTERS_SIZE     128
#define HRAM_SIZE             128

// I/O Ranges (DMG and CGB)
// Gives starts for segments of memory

#define JOYPAD         0xFF00
#define SERIAL         0xFF01
#define TIMER_DIVIDER  0xFF04
#define INTERRUPTS     0xFF0F
#define AUDIO          0xFF10
#define WAVE_PATTERN   0xFF30
#define LCD            0xFF40
#define OAM_DMA        0xFF46
#define BOOT_ROM_MAP   0xFF50

// Hardware registers (for both, some are exclusive to CGB)
#define SB    0xFF01
#define SC    0xFF02
#define DIV   0xFF04
#define TIMA  0xFF05
#define TMA   0xFF06
#define TAC   0xFF07
#define IF    0xFF0F

#define AUDIO_REG_START 0xFF10
#define AUDIO_REG_SIZE 23

#define LCDC  0xFF40
#define STAT  0xFF41
#define SCY   0xFF42
#define SCX   0xFF43
#define LY    0xFF44
#define LYC   0xFF45
#define DMA   0xFF46
#define BGP   0xFF47
#define OBP0  0xFF48
#define OBP1  0xFF49
#define WY    0xFF4A
#define WX    0xFF4B
#define KEY0  0xFF4C
#define SYS   0xFF4C
#define KEY1  0xFF4D
#define SPD   0xFF4D
#define VBK   0xFF4F
#define BANK  0xFF50
#define HDMA1 0xFF51
#define HDMA2 0xFF52
#define HDMA3 0xFF53
#define HDMA4 0xFF54
#define HDMA5 0xFF55
#define RP    0xFF56
#define BCPS  0xFF68
#define BGPI  0xFF68
#define BCPD  0xFF69
#define BGPD  0xFF69
#define OCPS  0xFF6A
#define OBPI  0xFF6A
#define OCPD  0xFF6B
#define OBPD  0xFF6B
#define OPRI  0xFF6C
#define SVBK  0xFF70
#define WBK   0xFF70
#define PCM12 0xFF76
#define PCM34 0xFF77

// I/O ranges (CGB only)
#define KEY_0_1        0xFF4C // KEY0 and KEY1
#define VRAM_BANK      0xFF4F
#define VRAM_DMA       0xFF51
#define IR_PORT        0xFF56
#define BG_OBJ_PALETTE 0xFF68
#define OBJ_PRIORITY   0xFF6C
#define WRAM_BANK      0xFF70

// Cartridge header

#define ENTRY_POINT       0x0100
#define NINTENDO_LOGO     0x0104
#define TILE              0x0134
#define MANUFACTURER_CODE 0x013F
#define CGB_FLAG          0x0143
#define NEW_LICENSEE_CODE 0x0144
#define SGB_FLAG          0x0146
#define CARTRIDGE_TYPE    0x0147
#define ROM_SIZE          0x0148
#define RAM_SIZE          0x0149
#define DESTINATION_CODE  0x014A
#define OLD_LICENSEE_CODE 0x014B
#define MASK_ROM_VERSION  0x014C
#define HEADER_CHECKSUM   0x014D
#define GLOBAL_CHECKSUM   0x014E
