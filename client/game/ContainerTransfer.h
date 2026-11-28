#pragma once

#include <string>

namespace game {

enum class TransferDirection {
	ToInventory,
	ToContainer,
};

struct ContainerTransfer {
	std::string containerId;
	TransferDirection direction;
	int slotIndex;
	int amount;
};

}
