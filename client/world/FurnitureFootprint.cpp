#include "world/FurnitureFootprint.h"

namespace world {

FurnitureFootprint::FurnitureFootprint(const GridSpan& span) : span_(span) {}

std::vector<GridCell> FurnitureFootprint::cellsAt(const GridCell& base) const {
	std::vector<GridCell> cells;
	for (int up = 0; up < span_.rows; ++up) {
		for (int right = 0; right < span_.columns; ++right) {
			cells.push_back(GridCell{base.column + right, base.row - up});
		}
	}
	return cells;
}

}
