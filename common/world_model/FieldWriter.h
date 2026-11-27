#pragma once

#include <iosfwd>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include <toml++/toml.hpp>

#include "world_model/Tile.h"

namespace worldmodel {

// Armado de valores TOML compartido por los writers del modelo.
class FieldWriter {
public:
	toml::array writePair(int first, int second) const;
	toml::table writeTilesets(
		const std::map<std::string, TilesetDef>& tilesets) const;
	std::string formatKey(std::string_view key) const;
	void appendBoolGrid(std::ostream& out, std::string_view key,
			    const std::vector<bool>& values,
			    int columns) const;
	void appendTileGrid(std::ostream& out, std::string_view key,
			    const TileGrid& grid, int columns) const;
	void appendOptionalTileGrid(std::ostream& out, std::string_view key,
				    const TileGrid& grid, int columns) const;

private:
	bool isBareKey(std::string_view key) const;
	void appendGrid(std::ostream& out, std::string_view key,
			const std::vector<std::string>& items,
			int columns) const;
};

}
