#include "commands.h"

namespace commands {

void apply(World& world, uint8_t player_id, const protocol::GiftResponseMessage& message) {
    if (message.accept) {
        // player_id es el receptor (el que apretó Aceptar)
        // message.sender_id es el emisor original (el que tenía el ítem en la mano)
        world.process_gift_acceptance(message.sender_id, player_id);
    } else {
        world.cancel_gift_request(message.sender_id, player_id);
    }
    // Aviso en los DOS casos: el emisor tiene el popup de espera abierto
    // hasta que llegue la respuesta, acepte o rechace el receptor.
    world.notify_gift_answered(message.sender_id);
}
}  // namespace commands
