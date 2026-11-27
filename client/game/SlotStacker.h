#pragma once

#include <string>
#include <vector>

#include "game/LocalWorldView.h"

namespace game {

class SlotStacker {
public:
	int add(std::vector<ViewInventorySlot>& slots,
		const std::string& contentId, int amount) const;

	bool remove(std::vector<ViewInventorySlot>& slots,
		    const std::string& contentId, int amount) const;

	const ViewInventorySlot* find(
		const std::vector<ViewInventorySlot>& slots,
		int slotIndex) const;
};

}
