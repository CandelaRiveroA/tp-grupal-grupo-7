#pragma once

#include <vector>

#include "common/common_game_direction.h"
#include "world/CreatureCatalog.h"
#include "world/ItemCatalog.h"

namespace game {


struct ViewEntity {
	world::CreatureKind kind;
	int cellX;
	int cellY;
	protocol::Direction facing;
	bool moving;
	world::ActionKind action = world::ActionKind::Idle;
};

struct ViewInventorySlot {
	int slotIndex;
	world::ItemKind item;
};

struct LocalWorldView {
	std::vector<ViewEntity> entities;
	std::vector<ViewInventorySlot> inventory;
};

}
