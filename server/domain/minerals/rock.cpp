#include "rock.h"

#include "../map/world.h"

Rock::Rock(uint16_t x, uint16_t y): Mineral(x, y, 1) {}

void Rock::mine_mineral(World& w) {
    // Sin drop: solo desaparece. Desde acá this queda destruido.
    w.remove_entity(this);
}

ItemID Rock::get_drop_id() const { return ItemID::ROCK; }
