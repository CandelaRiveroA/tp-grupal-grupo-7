#include "world_model/Tile.h"

namespace worldmodel {

std::string TileRef::key() const {
	return tileset + ":" + std::to_string(column) + "," +
	       std::to_string(row);
}

bool operator==(const TilesetDef& left, const TilesetDef& right) {
	return left.image == right.image && left.cellWidth == right.cellWidth &&
	       left.cellHeight == right.cellHeight;
}

bool operator==(const TileRef& left, const TileRef& right) {
	return left.tileset == right.tileset && left.column == right.column &&
	       left.row == right.row;
}

}
