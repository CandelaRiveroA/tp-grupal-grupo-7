#include "game/NpcPlacementResolver.h"

namespace game {

world::NpcCells NpcPlacementResolver::cellsFrom(
	const LocalWorldView& view) const {
	world::NpcCells cells;
	for (const ViewNpcPlacement& placement : view.npcPlacements) {
		cells[placement.id] =
			world::GridCell{placement.cellX, placement.cellY};
	}
	return cells;
}

}
