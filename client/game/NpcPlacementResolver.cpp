#include "game/NpcPlacementResolver.h"

namespace game {

world::NpcCells NpcPlacementResolver::cellsFrom(
	const LocalWorldView& view,
	const world::SceneDescription& scene) const {
	// Primero lo que dice el mapa; lo que manda la fuente lo pisa.
	world::NpcCells cells = scene.npcCells;
	for (const ViewNpcPlacement& placement : view.npcPlacements) {
		cells[placement.id] =
			world::GridCell{placement.cellX, placement.cellY};
	}
	return cells;
}

}
