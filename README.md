To build:
1. Install SDL3 and CMake onto your system
2. Download the emulator and unzip. Navigate to the folder using ``cd``
3. To build: ``mkdir build ; cd build ; cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native -flto" .. ; make ; cd -``

To run:
Before running, if you would like to use a gamepad, please connect this to your system. You will be prompted to decide whether to use the gamepad or not in the command line dialogue.

1. Navigate to the root directory and run: ``./build/kctboy``
2. You will be prompted with a dialogue in the command line to start a selected ROM. You must provide the name of the ROM-file without the `.gb` extension
3. You can create or select a save to load up and save to if loading a ROM for a game that originally had a battery-backed cartridge

To add ROMs to the emulator (this has to be done manually):
1. ROMs must end with the `.gb` extension to be usable (if this extension is uppercase, please replace with lowercase letters)
2. Rename the ROM such that there are no spaces in the name (e.g. change `Super Mario Land.gb` to `mario.gb`)


