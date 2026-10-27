#pragma once

#include <vector>

#include "common/common_game_direction.h"
#include "world/CreatureCatalog.h"
#include "world/Grid.h"
#include "world/ItemCatalog.h"

namespace game {


struct ViewEntity {
	world::CreatureKind kind;
	int cellX;
	int cellY;
	protocol::Direction facing;
	bool moving;
	world::ActionKind action = world::ActionKind::Idle;
	int id = 0;
};

struct ViewInventorySlot {
	int slotIndex;
	world::ItemKind item;
};

struct ViewGroundItem {
	int id;
	int cellX;
	int cellY;
	world::ItemKind item;
};

struct LocalWorldView {
	std::vector<ViewEntity> entities;
	std::vector<ViewGroundItem> groundItems;
	std::vector<ViewInventorySlot> inventory;
	world::GridSpan grid{0, 0};
};

}
