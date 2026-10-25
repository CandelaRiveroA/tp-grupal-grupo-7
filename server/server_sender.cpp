#include "server_sender.h"

ServerSender::ServerSender(ProtocolSender& protocol, Queue<ServerStatus>& statuses):
        protocol(protocol), statuses(statuses) {}

void ServerSender::execute() {
    while (should_keep_running()) {
        const ServerStatus status = statuses.pop();
        protocol.send_server_status(status);
    }
}
