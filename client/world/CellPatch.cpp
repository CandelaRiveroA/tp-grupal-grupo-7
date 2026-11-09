#include "world/CellPatch.h"

#include "world/WorldConstants.h"

namespace world {

CellPatch::CellPatch(const std::vector<GridCell>& cells) {
	for (const GridCell& cell : cells) {
		cells_.insert({cell.column, cell.row});
	}
}

bool CellPatch::contains(const GridCell& cell) const {
	return cells_.count({cell.column, cell.row}) > 0;
}

std::uint8_t CellPatch::maskAt(const GridCell& cell) const {
	std::uint8_t mask = 0;
	if (contains(GridCell{cell.column, cell.row - 1})) {
		mask |= kNeighborNorth;
	}
	if (contains(GridCell{cell.column + 1, cell.row})) {
		mask |= kNeighborEast;
	}
	if (contains(GridCell{cell.column, cell.row + 1})) {
		mask |= kNeighborSouth;
	}
	if (contains(GridCell{cell.column - 1, cell.row})) {
		mask |= kNeighborWest;
	}
	return mask;
}

}
