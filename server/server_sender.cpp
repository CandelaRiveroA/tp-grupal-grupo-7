#include "server_sender.h"

ServerSender::ServerSender(ProtocolSender& protocol, Queue<ServerMessage>& messages):
        protocol(protocol), messages(messages) {}

void ServerSender::execute() {
    while (should_keep_running()) {
        const ServerMessage message = messages.pop();
        protocol.send_server_message(message);
    }
}
