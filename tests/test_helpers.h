#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "../server/domain/map/world.h"
#include "../server/domain/entities/player.h"
#include "../server/domain/inventory/item_types.h"

bool has_item(const Player& player, ItemID id);
bool cell_has_item(World& world, uint16_t x, uint16_t y, ItemID id);
bool cell_has_furniture(const World& world, uint16_t x, uint16_t y);

#endif // TEST_HELPERS_H