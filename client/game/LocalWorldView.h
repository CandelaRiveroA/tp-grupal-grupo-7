#pragma once

#include <vector>

#include "world/CreatureCatalog.h"
#include "world/ItemCatalog.h"
#include "world/ViewDirection.h"

namespace game {


struct ViewEntity {
    world::CreatureKind kind;
    int cellX;
    int cellY;
    world::Direction facing;
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

}  // namespace game
