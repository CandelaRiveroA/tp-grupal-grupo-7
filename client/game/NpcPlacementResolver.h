#pragma once

#include "game/LocalWorldView.h"
#include "world/NpcCatalog.h"
#include "world/SceneDescription.h"

namespace game {

// Traduce las posiciones de NPCs que mandó la fuente de estado
// (LocalWorldView::npcPlacements) a las celdas que usan el dibujo y el click
// (world::NpcCells). Es el único lugar que conoce ese contrato: cuando el
// servidor mande las posiciones, lo único que cambia es quién llena la
// vista. Sin posiciones de la fuente, cada NPC queda en la celda que le
// da el mapa (el marker del editor) y, si tampoco, en su `at` del TOML.
class NpcPlacementResolver {
public:
	world::NpcCells cellsFrom(const LocalWorldView& view,
				  const world::SceneDescription& scene) const;
};

}
