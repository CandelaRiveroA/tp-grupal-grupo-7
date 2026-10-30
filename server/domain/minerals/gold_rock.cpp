#include "gold_rock.h"

#include <memory>
#include <utility>

#include "../entities/dropped_item.h"
#include "../inventory/product.h"
#include "../map/world.h"

GoldRock::GoldRock(uint16_t x, uint16_t y): Mineral(x, y, 4) {}

void GoldRock::mine_mineral(World& w) {
    uint16_t rx = this->pos_x;
    uint16_t ry = this->pos_y;

    auto orb = std::make_unique<Product>(ItemID::GOLD_ORB, 750, Quality::BASE);
    auto drop = std::make_unique<DroppedItem>(std::move(orb), rx, ry);

    w.set_entity_at(rx, ry, std::move(drop));
}

ItemID GoldRock::get_drop_id() const { return ItemID::GOLD_ORB; }
