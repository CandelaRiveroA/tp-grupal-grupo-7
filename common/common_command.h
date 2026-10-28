#ifndef COMMON_COMMAND_H
#define COMMON_COMMAND_H

#include <cstdint>
#include <variant>

#include "common_constants.h"

namespace protocol {

enum class ClientMessageType : uint8_t { Move = 1, Pick = 2, Take = 3 };

// Move: camina en la dirección dada (y la fija como facing).
struct MoveMessage {
    Direction direction;
};

// Pick: usa la herramienta equipada sobre la celda que el jugador tiene enfrente.
struct PickMessage {
    Direction direction;
};

// Take: recoge el item caído sobre la celda propia del jugador.
struct TakeMessage {
    Direction direction;
};

using ClientMessage = std::variant<MoveMessage, PickMessage, TakeMessage>;

}  // namespace protocol

#endif
