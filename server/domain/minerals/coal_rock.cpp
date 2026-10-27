#include "coal_rock.h"
#include "../entities/dropped_item.h"
#include "../inventory/product.h"
#include "../map/world.h"

CoalRock::CoalRock(uint16_t x, uint16_t y) : Mineral(x, y, 1) {}

void CoalRock::mine_mineral(Player& /*p*/, World& w) {
    uint16_t rx = this->pos_x;
    uint16_t ry = this->pos_y;

    auto orb = std::make_unique<Product>(ItemID::COAL_ORB, 15, Quality::BASE);
    auto drop = std::make_unique<DroppedItem>(std::move(orb), rx, ry);

    w.set_entity_at(rx, ry, std::move(drop));
}