#include "game/FarmSelection.h"

namespace game {

FarmSelection::FarmSelection(const world::ContentTraits& traits)
	: carry_(traits) {}

void FarmSelection::observe(const LocalWorldView& view, const int activeSlot,
			    const world::GridCell& hovered) {
	carry_.observe(view, activeSlot);
	planting_.observe(view, carry_, hovered);
}

const CarrySelection& FarmSelection::carry() const {
	return carry_;
}

const std::optional<PlantingSpot>& FarmSelection::spot() const {
	return planting_.spot();
}

}
