#include "silver_rock.h"
#include "../entities/dropped_item.h"
#include "../inventory/product.h"
#include "../map/world.h"
#include <cstdlib>

SilverRock::SilverRock(uint16_t x, uint16_t y) : Mineral(x, y, 3) {}

void SilverRock::mine_mineral(Player& p, World& w) {
    uint16_t rx = this->pos_x;
    uint16_t ry = this->pos_y;
    //TODO: Separar logica de randomizado?
    int chance = std::rand() % 100;
    
    if (chance < 10) {
        // Regla del 10%: Spawnear manada de orcos
        // TODO: w.spawn_orc_pack(rx, ry);
        w.remove_entity_at(rx, ry);
    } else {
        auto orb = std::make_unique<Product>(ItemID::SILVER_ORB, 250, Quality::BASE);
        auto drop = std::make_unique<DroppedItem>(std::move(orb), rx, ry);
        w.set_entity_at(rx, ry, std::move(drop));
    }
}