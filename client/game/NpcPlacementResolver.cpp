#include "game/NpcPlacementResolver.h"

#include "common/common_constants.h"
#include "game/AreaProximity.h"
#include "game/GameConstants.h"

namespace game {

world::NpcCells NpcPlacementResolver::cellsFrom(
	const LocalWorldView& view,
	const world::SceneDescription& scene) const {
	// Primero lo que dice el mapa; lo que manda la fuente lo pisa.
	world::NpcCells cells = scene.npcCells;
	for (const ViewNpcPlacement& placement : view.npcPlacements) {
		// La red manda los pies; el renderer y el mapa usan la esquina superior.
		cells[placement.id] = world::GridCell{
			placement.cellX, placement.cellY - kMerchantFeetOffset};
	}
	return cells;
}

bool NpcPlacementResolver::canInteractWith(const world::NpcDef& npc,
                                         const LocalWorldView& view,
                                         const world::NpcCells& cells) const {
    if (!npc.hasPopup()) {
        return true;
    }
    if (view.entities.empty()) {
        return false;
    }
    const world::GridCell top = npc.cellIn(cells);
    const ViewEntity& player = view.entities[kLocalPlayerIndex];
    return AreaProximity(kMerchantInteractionRadius).around(
            {top.column, top.row + kMerchantFeetOffset}, {1, 1},
            player.cellX, player.cellY);
}

}
