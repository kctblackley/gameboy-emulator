A Gameboy emulator that can currently pass all Blargg tests (except 2, as I am currently implementing interrupts, timers, and the PPU).
No AI was used throughout the entire creation of this project.

(Rough README, ignore how unprofessional it is at the moment)

To use:
1. Download the code
2. Install SDL2 onto your system (if not yet installed)
3. In the command line, cd to the gameboy-emulator folder
4. Run: g++ *.cpp -lSDL2
5. Then go online and download the ROM you want to play. Create a folder entitled 'rom' within the gameboy-emulator folder.
6. Then run: ./a.out (name of rom here) 1 -> the 1 is needed as I haven't yet altered how the program reads command line arguments (this will be changed later)

A window should open with a 'shaking' Nintendo logo (this is the unofficial boot-rom, as the official one seen on actual Game Boy games is copyrighted). Afterwards, the game should run.

This is an imperfect emulator, so visual artifacts will occur and some games may not run at all (notable example is Bomb Jack at the time of writing this, and Road Rash which appeared to pause at one of the screens and not enter gameplay).
