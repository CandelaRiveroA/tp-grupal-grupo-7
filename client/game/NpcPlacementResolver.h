#pragma once

#include "game/LocalWorldView.h"
#include "world/NpcCatalog.h"

namespace game {

// Traduce las posiciones de NPCs que mandó la fuente de estado
// (LocalWorldView::npcPlacements) a las celdas que usan el dibujo y el click
// (world::NpcCells). Es el único lugar que conoce ese contrato: cuando el
// servidor mande las posiciones, lo único que cambia es quién llena la
// vista. Sin posiciones, cada NPC queda en su `at` por defecto del TOML.
class NpcPlacementResolver {
public:
	world::NpcCells cellsFrom(const LocalWorldView& view) const;
};

}
