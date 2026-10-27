#include "lapis_rock.h"
#include "../entities/dropped_item.h"
#include "../inventory/product.h"
#include "../map/world.h"
#include <cstdlib>

LapisRock::LapisRock(uint16_t x, uint16_t y) : Mineral(x, y, 2) {}

void LapisRock::mine_mineral(World& w) {
    uint16_t rx = this->pos_x;
    uint16_t ry = this->pos_y;

    int chance = std::rand() % 100;
    
    if (chance <= 15) { 
        w.remove_entity_at(rx, ry);
    } else {
        auto orb = std::make_unique<Product>(ItemID::LAPIS_ORB, 100, Quality::BASE);
        auto drop = std::make_unique<DroppedItem>(std::move(orb), rx, ry);
        w.set_entity_at(rx, ry, std::move(drop));
    }
}