#include "game/SlotStacker.h"

#include <algorithm>

namespace game {

int SlotStacker::add(std::vector<ViewInventorySlot>& slots,
		     const std::string& contentId, const int amount) const {
	for (ViewInventorySlot& slot : slots) {
		if (slot.contentId == contentId) {
			slot.count += amount;
			return slot.slotIndex;
		}
	}
	int index = 0;
	while (find(slots, index) != nullptr) {
		++index;
	}
	slots.push_back({index, contentId, amount});
	return index;
}

bool SlotStacker::remove(std::vector<ViewInventorySlot>& slots,
			 const std::string& contentId,
			 const int amount) const {
	const auto found = std::find_if(
		slots.begin(), slots.end(),
		[&](const ViewInventorySlot& slot) {
			return slot.contentId == contentId;
		});
	if (found == slots.end() || found->count < amount) {
		return false;
	}
	found->count -= amount;
	if (found->count == 0) {
		slots.erase(found);
	}
	return true;
}

const ViewInventorySlot* SlotStacker::find(
	const std::vector<ViewInventorySlot>& slots,
	const int slotIndex) const {
	for (const ViewInventorySlot& slot : slots) {
		if (slot.slotIndex == slotIndex) {
			return &slot;
		}
	}
	return nullptr;
}

}
