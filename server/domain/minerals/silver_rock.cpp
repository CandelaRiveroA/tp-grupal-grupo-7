#include "silver_rock.h"

#include <cstdlib>
#include <memory>
#include <utility>

#include "../entities/dropped_item.h"
#include "../enemies/orc.h"
#include "../inventory/product.h"
#include "../map/world.h"

SilverRock::SilverRock(uint16_t x, uint16_t y): Mineral(x, y, 3) {}

void SilverRock::mine_mineral(World& w) {
    uint16_t rx = this->pos_x;
    uint16_t ry = this->pos_y;
    // TODO: Separar logica de randomizado?
    int chance = std::rand() % 100;

    // La roca se rompe siempre; desde acá this queda destruido (solo rx y ry).
    w.remove_entity(this);
    if (chance < 10) {
        // Regla del 10%: Spawnear manada de orcos
        w.spawn_orc_pack(rx, ry);
    } else {
        auto orb = std::make_unique<Product>(ItemID::SILVER_ORB, 250, Quality::BASE);
        auto drop = std::make_unique<DroppedItem>(std::move(orb), rx, ry);
        w.set_entity_at(rx, ry, std::move(drop));
    }
}

ItemID SilverRock::get_drop_id() const { return ItemID::SILVER_ORB; }
