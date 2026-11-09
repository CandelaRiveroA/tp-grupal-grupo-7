#include "world_model/Geometry.h"

namespace worldmodel {

bool operator==(const TilePos& left, const TilePos& right) {
	return left.column == right.column && left.row == right.row;
}

bool operator<(const TilePos& left, const TilePos& right) {
	if (left.row != right.row) {
		return left.row < right.row;
	}
	return left.column < right.column;
}

}
