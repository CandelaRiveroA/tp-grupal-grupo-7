#include "client_receiver.h"

ClientReceiver::ClientReceiver(ProtocolReceiver& protocol, Queue<ServerStatus>& statuses):
        protocol(protocol), statuses(statuses) {}

void ClientReceiver::execute() {
    while (should_keep_running()) {
        statuses.push(protocol.receive_server_status());
    }
}
