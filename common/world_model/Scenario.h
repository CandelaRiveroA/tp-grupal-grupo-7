#pragma once

#include <map>
#include <string>
#include <vector>

#include "world_model/Map.h"
#include "world_model/Tile.h"

namespace worldmodel {

struct Scenario {
	std::string id;
	MapKind kind = MapKind::Farm;
	int width = 0;
	int height = 0;
	int tileSize = 16;
	std::map<std::string, TilesetDef> tilesets;
	std::vector<bool> collision;
	TileGrid ground;
};

}
