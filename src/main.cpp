#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <vector>
#include "GB.h"

int main(int argc, char *argv[]) {
	std::string game;
	GB gameboy;
	struct stat sb;

	bool running = true;
	while(running) {
		bool load_save = false;
		std::string chosen_save = "";

		int hardware_mode = DMG_MODE;
		std::cout << "Enter hardware mode (0 = Game Boy, 1 = Game Boy Color)\n>> ";
		std::cin >> hardware_mode;
		if (hardware_mode != DMG_MODE && hardware_mode != CGB_MODE) {
			hardware_mode = DMG_MODE;
		}

		std::cout << "Enter a game name\n>> ";
		std::cin >> game;
		std::string directory = "rom/" + game + ".gb" + ( (hardware_mode == CGB_MODE) ? "c" : "");
		std::string saves = "saves/" + game + ( (hardware_mode == CGB_MODE) ? "_gbc" : "_gb");
		if (stat(saves.c_str(), &sb) == 0) {
			std::cout << "Saves for this game exist...\n";
			int i = 0;
			std::vector<std::string> save_files;
			for (const auto& e : std::filesystem::directory_iterator(saves)) {
				i++;
				std::string save_file = e.path().stem().string();
				save_files.push_back(save_file);
				std::cout << i << ". " << save_file << "\n";
			}
			std::cout << "Enter number to load save data (0 to create a new save)\n>> ";
			std::cin >> i;
			if (!(i == 0 || i > save_files.size())) {
				load_save = true;
				chosen_save = save_files[i - 1];
			}
		}
		if (stat(directory.c_str(), &sb) == 0) {
			gameboy.cpu.mmu.rom_name = game;
			gameboy.set_hardware_mode(hardware_mode);
			gameboy.run_rom(directory, load_save, chosen_save);
			running = false;
		} else {
			if (game == "exit") {
				running = false;
			} else {
				std::cout << "That game ROM does not exist\n";
			}
		}
	}
	return 0;
}