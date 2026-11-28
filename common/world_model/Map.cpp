#include "world_model/Map.h"

namespace worldmodel {

bool Map::contains(TilePos pos) const {
	return pos.column >= 0 && pos.row >= 0 && pos.column < width &&
	       pos.row < height;
}

bool Map::fits(TilePos origin, int rectWidth, int rectHeight) const {
	return rectWidth > 0 && rectHeight > 0 && origin.column >= 0 &&
	       origin.row >= 0 && origin.column + rectWidth <= width &&
	       origin.row + rectHeight <= height;
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

bool Map::isAreaWalkable(TilePos origin, int rectWidth, int rectHeight) const {
	if (!fits(origin, rectWidth, rectHeight)) {
		return false;
	}
	for (int row = 0; row < rectHeight; ++row) {
		for (int col = 0; col < rectWidth; ++col) {
			if (!isWalkable(
				    {origin.column + col, origin.row + row})) {
				return false;
			}
		}
	}
	return true;
}

std::vector<bool> Map::composeCollision() const {
	std::vector<bool> composed = baseCollision;
	for (const MapObject& object : objects) {
		const bool hasGrid =
			object.width > 0 && object.height > 0 &&
			object.collisionGrid.size() ==
				static_cast<std::size_t>(object.width) *
					static_cast<std::size_t>(object.height);
		if (!hasGrid && !object.blocked) {
			continue;
		}
		for (int row = 0; row < object.height; ++row) {
			for (int column = 0; column < object.width; ++column) {
				if (hasGrid) {
					const std::size_t cell =
						static_cast<std::size_t>(row) *
							static_cast<std::size_t>(
								object.width) +
						static_cast<std::size_t>(column);
					if (!object.collisionGrid[cell]) {
						continue;
					}
				}
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
