#include "game/MoveCommand.h"

namespace game {

protocol::ClientMessage MoveCommand::propose(world::Direction direction) const {
    return protocol::MoveMessage{world::wireDirectionFrom(direction)};
}

protocol::ClientMessage MoveCommand::use(world::Direction direction) const {
    return protocol::PickMessage{world::wireDirectionFrom(direction)};
}

protocol::ClientMessage MoveCommand::take(world::Direction direction) const {
    return protocol::TakeMessage{world::wireDirectionFrom(direction)};
}

}  // namespace game
