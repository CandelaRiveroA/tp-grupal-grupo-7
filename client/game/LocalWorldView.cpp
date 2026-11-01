#include "game/LocalWorldView.h"

namespace game {

int LocalWorldView::addInventoryContent(const std::string& contentId) {
	for (ViewInventorySlot& slot : inventory) {
		if (slot.contentId == contentId) {
			++slot.count;
			return slot.slotIndex;
		}
	}

	// No estaba: ocupar el primer slotIndex libre.
	int index = 0;
	bool used = true;
	while (used) {
		used = false;
		for (const ViewInventorySlot& slot : inventory) {
			if (slot.slotIndex == index) {
				used = true;
				++index;
				break;
			}
		}
	}
	inventory.push_back({index, contentId, 1});
	return index;
}

void LocalWorldView::placeInventoryContent(const int slotIndex,
					   const std::string& contentId,
					   const int count) {
	for (ViewInventorySlot& slot : inventory) {
		if (slot.contentId == contentId) {
			slot.slotIndex = slotIndex;
			slot.count = count;
			return;
		}
	}
	inventory.push_back({slotIndex, contentId, count});
}

bool LocalWorldView::inventorySlotFree(const int slotIndex) const {
	for (const ViewInventorySlot& slot : inventory) {
		if (slot.slotIndex == slotIndex) {
			return false;
		}
	}
	return true;
}

}  // namespace game
