#include "commands.h"

namespace commands {

void apply(World& world, uint8_t player_id, const protocol::SleepMessage& message) {
    world.answer_sleep(player_id, message.accept);
}

}  // namespace commands
