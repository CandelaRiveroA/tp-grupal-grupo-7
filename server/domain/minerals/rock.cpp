#include "rock.h"

#include <cstdlib>

#include "../map/world.h"

Rock::Rock(uint16_t x, uint16_t y):
        Mineral(x, y, 1),
        visual_id(static_cast<ItemID>(static_cast<uint16_t>(ItemID::ROCK) +
                                     std::rand() % 6)) {}

void Rock::mine_mineral(World& w) {
    // Sin drop: solo desaparece. Desde acá this queda destruido.
    w.remove_entity(this);
}

ItemID Rock::get_drop_id() const { return visual_id; }
