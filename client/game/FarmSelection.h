#pragma once

#include <optional>

#include "game/CarrySelection.h"
#include "game/LocalWorldView.h"
#include "game/PlantingSelection.h"
#include "world/ContentTraits.h"
#include "world/Grid.h"

namespace game {

// La presentación local del campo: lo que el jugador sostiene sobre la
// cabeza (CarrySelection) y dónde marcar el lugar de plantado
// (PlantingSelection) salen de la MISMA selección del HUD, así que se
// observan juntos y en orden (primero qué sostiene, después dónde
// marcar). Una sola llamada por tick; las consultas no calculan nada.
class FarmSelection {
public:
	explicit FarmSelection(const world::ContentTraits& traits);

	// activeSlot = slot activo del inventario; hovered = celda del
	// mundo bajo el puntero.
	void observe(const LocalWorldView& view, int activeSlot,
		     const world::GridCell& hovered);

	const CarrySelection& carry() const;

	const std::optional<PlantingSpot>& spot() const;

private:
	CarrySelection carry_;
	PlantingSelection planting_;
};

}
