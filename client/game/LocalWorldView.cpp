#include "game/LocalWorldView.h"

#include "game/GameConstants.h"
#include "game/SlotStacker.h"

namespace game {

void LocalWorldView::addChatLine(const ViewChatLine& line) {
	chatLog.push_back(line);
	if (chatLog.size() > kChatHistoryMax) {
		chatLog.erase(chatLog.begin());
	}
	++chatVersion;
}

int LocalWorldView::addInventoryContent(const std::string& contentId) {
	return SlotStacker().add(inventory, contentId, 1);
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
