#include "client_receiver.h"

ClientReceiver::ClientReceiver(ProtocolReceiver& protocol, Queue<ServerMessage>& messages):
        protocol(protocol), messages(messages) {}

void ClientReceiver::execute() {
    while (should_keep_running()) {
        messages.push(protocol.receive_server_message());
    }
}
