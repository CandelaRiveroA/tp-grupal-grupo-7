#include "commands.h"

namespace commands {

void apply(World& world, uint8_t player_id, const protocol::PickMessage& message) {
    world.set_player_facing(player_id, message.direction);
    world.use_tool(player_id);
}

}  // namespace commands
