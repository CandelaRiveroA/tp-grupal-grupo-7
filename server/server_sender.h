#ifndef SERVER_SENDER_H
#define SERVER_SENDER_H

#include "common_protocol_sender.h"
#include "common_queue.h"
#include "common_server_status.h"
#include "common_worker.h"

class ServerSender: public Worker {
private:
    ProtocolSender& protocol;
    Queue<ServerStatus>& statuses;

public:
    ServerSender(ProtocolSender& protocol, Queue<ServerStatus>& statuses);

protected:
    void execute() override;
};

#endif
