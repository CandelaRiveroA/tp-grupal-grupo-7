#include "commands.h"

namespace commands {

void apply(World& world, uint8_t player_id, const protocol::GiftResponseMessage& message) {
    if (message.accept) {
        // player_id es el receptor (el que apretó Aceptar)
        // message.sender_id es el emisor original (el que tenía el ítem en la mano)
        world.process_gift_acceptance(message.sender_id, player_id);
    } else {
        // TODO: Notificar al emisor que el regalo fue rechazado.
    }
}
}  // namespace commands
