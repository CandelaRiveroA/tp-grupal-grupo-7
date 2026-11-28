#include "game/FurnitureOrientation.h"

namespace game {

void FurnitureOrientation::observe(const CarrySelection& carry, const int activeSlot) {
	const std::optional<std::string> held =
		carry.heldFootprint().has_value() ? carry.held() : std::nullopt;
	if (held != held_ || activeSlot != slot_) {
		facing_ = world::FurnitureFacing::Front;
	}
	held_ = held;
	slot_ = activeSlot;
}

void FurnitureOrientation::rotate(const world::FurnitureViews& views) {
	if (held_.has_value()) {
		facing_ = views.next(*held_, facing_);
	}
}

void FurnitureOrientation::reset() { facing_ = world::FurnitureFacing::Front; }

world::FurnitureFacing FurnitureOrientation::facing() const { return facing_; }

}
