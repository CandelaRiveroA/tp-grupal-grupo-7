#pragma once

#include <array>
#include <vector>

namespace world {

// Colores RGB (0-255) del editor de personaje
struct CharacterPalettes {
	std::vector<std::array<int, 3>> eyes;
	std::vector<std::array<int, 3>> hair;
	std::vector<std::array<int, 3>> pants;
};

}
