#ifndef COMMON_COMMAND_H
#define COMMON_COMMAND_H

#include <cstdint>
#include <string>
#include <variant>

#include "common_constants.h"

namespace protocol {

enum class ClientMessageType : uint8_t { Move = 1, Pick = 2, Take = 3, Sleep = 4, Wake = 5, Chat = 6, GiftResponse = 7 };

// Move: camina en la dirección dada (y la fija como facing).
struct MoveMessage {
    Direction direction;
};

// Pick: usa la herramienta del casillero `slot` sobre la celda enfrentada.
// La selección del inventario es local; viaja solo al usar.
struct PickMessage {
    Direction direction;
    uint8_t slot;
};

// Take: recoge el item caído sobre la celda propia del jugador.
struct TakeMessage {
    Direction direction;
};

// Sleep: responde al aviso de la cama. accept = true confirma dormir (y
// avanzar de día si ya aceptaron todos); false cancela: sale de la cama y
// recupera el movimiento.
struct SleepMessage {
    bool accept;
};

// Wake: apretó OK en el popup del nuevo día. El día arranca cuando todos lo mandaron.
struct WakeMessage {};

// Chat: texto libre del jugador. Puede ser mensaje público, privado (@nombre) o
// un comando que empieza con '/'. El parseo y el routing los hace el server.
struct ChatMessage {
    std::string text;
};

// GiftResponse: respuesta a un regalo de item de otro jugador. accept = true
// confirma recibirlo (y lo suma al inventario); false cancela: no lo recibe.
struct GiftResponseMessage {
    uint8_t sender_id;
    bool accept;
};

using ClientMessage =
        std::variant<MoveMessage, PickMessage, TakeMessage, SleepMessage, WakeMessage, ChatMessage, GiftResponseMessage>;

}  // namespace protocol

#endif
