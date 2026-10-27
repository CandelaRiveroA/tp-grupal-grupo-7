#ifndef COMMON_COMMAND_H
#define COMMON_COMMAND_H

#include <cstdint>
#include <variant>

#include "common_constants.h"

namespace protocol {

enum class ClientMessageType : uint8_t { Move = 1 };

struct MoveMessage {
    Direction direction;
};

using ClientMessage = std::variant<MoveMessage>;

}  // namespace protocol

#endif
