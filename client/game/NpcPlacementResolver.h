#pragma once

#include "game/LocalWorldView.h"
#include "world/NpcCatalog.h"
#include "world/SceneDescription.h"

namespace game {

// Traduce las posiciones de NPCs que mandó la fuente de estado
// (LocalWorldView::npcPlacements) a las celdas que usan el dibujo y el click
// (world::NpcCells). La red usa los pies; el mapa y el dibujo, la esquina
// superior. Esta clase concentra la conversión y la cercanía de interacción.
// Sin posiciones de la fuente, cada NPC queda en la celda que le
// da el mapa (el marker del editor) y, si tampoco, en su `at` del TOML.
class NpcPlacementResolver {
public:
    // Misma regla para el click y el cursor. Los objetos sin popup conservan
    // su interacción visual; los mercaderes requieren cercanía a los pies.
    bool canInteractWith(const world::NpcDef& npc, const LocalWorldView& view,
                         const world::NpcCells& cells) const;

	world::NpcCells cellsFrom(const LocalWorldView& view,
				  const world::SceneDescription& scene) const;
};

}
