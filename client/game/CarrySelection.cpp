#include "game/CarrySelection.h"

namespace game {

CarrySelection::CarrySelection(const world::ContentTraits& traits)
	: traits_(traits) {}

void CarrySelection::observe(const LocalWorldView& view,
			     const int activeSlot) {
	held_.reset();
	for (const ViewInventorySlot& slot : view.inventory) {
		if (slot.slotIndex == activeSlot &&
		    traits_.isDroppable(slot.contentId)) {
			held_ = slot.contentId;
			return;
		}
	}
}

const std::optional<std::string>& CarrySelection::held() const {
	return held_;
}

}
