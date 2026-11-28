#include "game/PlantingSelection.h"

#include "game/GameConstants.h"

namespace game {

void PlantingSelection::observe(const LocalWorldView& view,
				const CarrySelection& carry,
				const world::GridCell& hovered) {
	spot_.reset();
	// Un mueble en la mano se coloca (PlacementSelection), no se siembra.
	if (!carry.held().has_value() || carry.heldFootprint().has_value()) {
		return;
	}
	// Solo se marca donde un click siembra exactamente esa celda (el
	// mismo alcance que PlayerController::useToward).
	if (!isReachable(view, hovered)) {
		return;
	}
	for (const ViewCrop& crop : view.plantedCrops) {
		// La celda ya tiene su cultivo: el marcador se pone
		// "ocupado" (el cuadrado rojo) en vez de disponible.
		if (crop.cellX == hovered.column && crop.cellY == hovered.row) {
			spot_ = PlantingSpot{hovered,
					     world::BuildPlaceState::Placed};
			return;
		}
	}
	for (const ViewGroundEffect& effect : view.groundEffects) {
		// La tierra tratada (Dug) es la que se puede sembrar: el
		// marcador "disponible" va sobre ella. Otras consecuencias
		// (regar) no habilitan plantar por sí solas.
		if (effect.cellX == hovered.column &&
		    effect.cellY == hovered.row &&
		    effect.consequence == world::GroundConsequence::Dug) {
			spot_ = PlantingSpot{hovered,
					     world::BuildPlaceState::Available};
			return;
		}
	}
}

const std::optional<PlantingSpot>& PlantingSelection::spot() const {
	return spot_;
}

bool PlantingSelection::isReachable(const LocalWorldView& view,
				    const world::GridCell& hovered) const {
	if (view.entities.empty()) {
		return false;
	}
	const ViewEntity& player = view.entities[kLocalPlayerIndex];
	const int dx = hovered.column - player.cellX;
	const int dy = hovered.row - player.cellY;
	// Las 4 celdas vecinas en eje: ni diagonales ni la propia.
	return (dx == 0 && (dy == 1 || dy == -1)) ||
	       (dy == 0 && (dx == 1 || dx == -1));
}

}
