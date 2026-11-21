#include "commands.h"

namespace commands {

void apply(World& world, uint8_t player_id, const protocol::TakeMessage& message) {
    world.set_player_facing(player_id, message.direction);
    // La selección del hotbar es local: sin esto el server tendría el
    // casillero viejo y no sabría qué tiene equipado (el regalo lo pide).
    world.select_slot(player_id, message.slot);
    world.take_item(player_id);
}

}  // namespace commands
