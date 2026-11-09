#pragma once

#include <map>
#include <string>

#include "world_model/Map.h"
#include "world_model/Tile.h"

namespace worldmodel {

struct World {
	int formatVersion = 1;
	std::string slug;
	std::string name;
	std::string description;
	std::map<std::string, TilesetDef> tilesets;
	std::map<std::string, Map> maps;
};

}
