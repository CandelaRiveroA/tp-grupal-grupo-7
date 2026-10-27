#include "client_sender.h"

ClientSender::ClientSender(ProtocolSender& protocol, Queue<protocol::ClientMessage>& commands):
        protocol(protocol), commands(commands) {}

void ClientSender::execute() {
    while (should_keep_running()) {
        const protocol::ClientMessage message = commands.pop();
        protocol.send_client_message(message);
    }
}
