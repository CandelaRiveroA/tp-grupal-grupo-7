#pragma once

#include <optional>
#include <vector>

#include "world_model/Geometry.h"
#include "world_model/Tile.h"

namespace worldmodel {

enum class ObjectRole {
	Cabin,
	Market,
	CommunityCenter,
	Decoration,
};

struct MapObject {
	ObjectRole role = ObjectRole::Decoration;
	TileRef tile;
	TilePos cell;
	int width = 1;
	int height = 1;
	bool blocked = false;
	std::optional<TilePos> door;   // celda absoluta en este mapa
	std::vector<bool> collisionGrid;
};

}
