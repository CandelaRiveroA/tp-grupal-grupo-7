#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "world_model/Geometry.h"
#include "world_model/MapObject.h"
#include "world_model/Marker.h"
#include "world_model/Portal.h"
#include "world_model/Tile.h"

namespace worldmodel {

enum class MapKind {
	Farm,
	Town,
	Cabin,
	Market,
	CommunityCenter,
	Dungeon,
};

struct Map {
	std::string id;
	MapKind kind = MapKind::Farm;
	int level = 0;   // solo Dungeon: 1..10 en obligatorios; resto: 0
	int width = 0;
	int height = 0;
	int tileSize = 16;
	std::vector<bool> baseCollision;
	std::vector<bool> collision;
	TileGrid ground;
	TileGrid paths;
	std::vector<MapObject> objects;
	std::vector<Marker> markers;
	std::vector<Portal> portals;

	bool contains(TilePos pos) const;
	std::size_t cellIndex(TilePos pos) const;
	bool isWalkable(TilePos pos) const;
	std::vector<bool> composeCollision() const;   // no muta
	void rebuildCollision();                      // collision = composeCollision()
};

}
