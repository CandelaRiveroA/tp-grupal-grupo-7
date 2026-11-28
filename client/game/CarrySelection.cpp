#include "game/CarrySelection.h"

namespace game {

CarrySelection::CarrySelection(const world::ContentTraits& traits)
	: traits_(traits) {}

void CarrySelection::observe(const LocalWorldView& view,
			     const int activeSlot) {
	held_.reset();
	heldFootprint_.reset();
	for (const ViewInventorySlot& slot : view.inventory) {
		if (slot.slotIndex == activeSlot &&
		    (traits_.isDroppable(slot.contentId) ||
		     slot.footprint.has_value())) {
			held_ = slot.contentId;
			heldFootprint_ = slot.footprint;
			return;
		}
	}
}

const std::optional<world::GridSpan>& CarrySelection::heldFootprint() const {
	return heldFootprint_;
}

const std::optional<std::string>& CarrySelection::held() const {
	return held_;
}

}
