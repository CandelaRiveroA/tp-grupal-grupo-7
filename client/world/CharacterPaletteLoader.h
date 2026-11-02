#pragma once

#include <array>
#include <string>
#include <vector>

#include "toml/TomlDocument.h"
#include "world/CharacterPalettes.h"

namespace world {

// Traduce character_palettes.toml a CharacterPalettes; lanza ConfigError si un
// color no tiene 3 canales entre 0 y 255.
class CharacterPaletteLoader {
public:
	CharacterPalettes loadFromFile(const std::string& path) const;

	CharacterPalettes loadFromString(const std::string& toml) const;

private:
	CharacterPalettes read(const tomlcfg::TomlDocument& doc) const;

	std::vector<std::array<int, 3>> readColors(
		const tomlcfg::TomlDocument& doc, const std::string& table) const;
};

}
