#include "game/InventorySelector.h"

namespace game {

InventorySelector::InventorySelector() : activeSlot_(kDefaultSlot) {}

void InventorySelector::select(int slot, int slotCount) {
	if (slot < 0 || slot >= slotCount) {
		return;
	}
	activeSlot_ = slot;
}

int InventorySelector::active() const {
	return activeSlot_;
}

}
