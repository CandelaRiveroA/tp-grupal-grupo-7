#include "coal_rock.h"

#include <memory>
#include <utility>

#include "../entities/dropped_item.h"
#include "../inventory/product.h"
#include "../map/world.h"

CoalRock::CoalRock(uint16_t x, uint16_t y): Mineral(x, y, 1) {}

void CoalRock::mine_mineral(World& w) {
    uint16_t rx = this->pos_x;
    uint16_t ry = this->pos_y;

    // Desde acá this queda destruido: solo se usan rx y ry.
    w.remove_entity(this);
    auto orb = std::make_unique<Product>(ItemID::COAL_ORB, 10, Quality::BASE);
    auto drop = std::make_unique<DroppedItem>(std::move(orb), rx, ry);

    w.set_entity_at(rx, ry, std::move(drop));
}

ItemID CoalRock::get_drop_id() const { return ItemID::COAL_ORB; }
