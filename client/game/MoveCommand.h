#pragma once

#include "common/common_command.h"
#include "world/ViewDirection.h"

namespace game {

class MoveCommand {
public:
    protocol::ClientMessage propose(world::Direction direction) const;
};

}  // namespace game
