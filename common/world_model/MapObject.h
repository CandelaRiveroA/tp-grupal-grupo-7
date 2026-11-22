#pragma once

#include <optional>
#include <vector>

#include "world_model/Geometry.h"
#include "world_model/Tile.h"

namespace worldmodel {

enum class ObjectRole {
	Cabin,
	Carpentry,
	Blacksmith,
	GeneralStore,
	Ranch,
	Barn,
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
	std::optional<TilePos> door;   // celda absoluta izquierda de la puerta
	std::vector<bool> collisionGrid;
	int doorWidth = 1;

	bool hitsDoor(TilePos pos) const {
		return door.has_value() && pos.row == door->row &&
		       pos.column >= door->column &&
		       pos.column < door->column + doorWidth;
	}
};

}
