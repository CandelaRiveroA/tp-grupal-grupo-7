#include "commands.h"

namespace commands {

void apply(World& world, uint8_t player_id, const protocol::MoveMessage& message) {
    world.move_player(player_id, message.direction);
}

void apply(World& world, uint8_t player_id, const protocol::ToggleWalkingMessage& /*message*/) {
    world.toggle_player_walking(player_id);
}

}  // namespace commands
