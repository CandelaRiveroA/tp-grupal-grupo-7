#ifndef CLIENT_RECEIVER_H
#define CLIENT_RECEIVER_H

#include "common_protocol_receiver.h"
#include "common_queue.h"
#include "common_server_status.h"
#include "common_worker.h"

class ClientReceiver: public Worker {
private:
    ProtocolReceiver& protocol;
    Queue<ServerStatus>& statuses;

public:
    ClientReceiver(ProtocolReceiver& protocol, Queue<ServerStatus>& statuses);

protected:
    void execute() override;
};

#endif
