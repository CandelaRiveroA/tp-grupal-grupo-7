#include "server_receiver.h"

#include <optional>

ServerReceiver::ServerReceiver(ProtocolReceiver& protocol, Queue<MatchEvent>& events,
                               uint8_t player_id):
        protocol(protocol), events(events), player_id(player_id) {}

void ServerReceiver::execute() {
    while (should_keep_running()) {
        std::optional<protocol::ClientMessage> message = protocol.receive_command();
        if (!message.has_value()) {
            return;
        }

        events.push(MatchEvent(PlayerMessage{player_id, message.value()}));
    }
}
