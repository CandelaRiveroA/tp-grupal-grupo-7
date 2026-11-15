#pragma once

#include <map>
#include <string>
#include <vector>

#include <toml++/toml.hpp>

#include "world_model/Tile.h"

namespace worldmodel {

// Armado de valores TOML compartido por los writers del modelo.
class FieldWriter {
public:
	static toml::array writePair(int first, int second);
	static toml::array writeBooleans(const std::vector<bool>& values);
	static toml::array writeGrid(const TileGrid& grid);
	static toml::table writeTilesets(
		const std::map<std::string, TilesetDef>& tilesets);
};

}
