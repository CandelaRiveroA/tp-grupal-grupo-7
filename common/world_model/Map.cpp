#include "world_model/Map.h"

namespace worldmodel {

bool Map::contains(TilePos pos) const {
	return pos.column >= 0 && pos.row >= 0 && pos.column < width &&
	       pos.row < height;
}

std::size_t Map::cellIndex(TilePos pos) const {
	return static_cast<std::size_t>(pos.row) *
		       static_cast<std::size_t>(width) +
	       static_cast<std::size_t>(pos.column);
}

bool Map::isWalkable(TilePos pos) const {
	if (!contains(pos)) {
		return false;
	}
	const std::size_t index = cellIndex(pos);
	if (index >= collision.size()) {
		return false;
	}
	return !collision[index];
}

std::vector<bool> Map::composeCollision() const {
	std::vector<bool> composed = baseCollision;
	for (const MapObject& object : objects) {
		if (!object.blocked) {
			continue;
		}
		for (int row = 0; row < object.height; ++row) {
			for (int column = 0; column < object.width; ++column) {
				const TilePos pos{object.cell.column + column,
						  object.cell.row + row};
				if (!contains(pos)) {
					continue;
				}
				const std::size_t index = cellIndex(pos);
				if (index < composed.size()) {
					composed[index] = true;
				}
			}
		}
	}
	return composed;
}

void Map::rebuildCollision() { collision = composeCollision(); }

}
