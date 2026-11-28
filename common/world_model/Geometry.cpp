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

bool withinInteractionRange(TilePos origin, int width, int height, TilePos point, int ring) {
    return point.column >= origin.column - ring &&
           point.column <= origin.column + width - 1 + ring &&
           point.row >= origin.row - ring &&
           point.row <= origin.row + height - 1 + ring;
}

}
