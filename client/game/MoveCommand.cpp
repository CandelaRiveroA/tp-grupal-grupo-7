#include "game/MoveCommand.h"

namespace game {

protocol::ClientMessage MoveCommand::propose(world::Direction direction) const {
    return protocol::MoveMessage{world::wireDirectionFrom(direction)};
}

}  // namespace game
