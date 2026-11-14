#pragma once

#include <optional>
#include <string>
#include <vector>

namespace worldmodel {

struct TilesetDef {
	std::string image;
	int cellWidth = 0;
	int cellHeight = 0;
};

struct TileRef {
	std::string tileset;
	int column = 0;
	int row = 0;

	std::string key() const;
};

bool operator==(const TilesetDef& left, const TilesetDef& right);
bool operator==(const TileRef& left, const TileRef& right);

using TileGrid = std::vector<std::optional<TileRef>>;

}
