#ifndef CLIENT_SENDER_H
#define CLIENT_SENDER_H

#include "common_command.h"
#include "common_protocol_sender.h"
#include "common_queue.h"
#include "common_worker.h"

class ClientSender: public Worker {
private:
    ProtocolSender& protocol;
    Queue<protocol::ClientMessage>& commands;

public:
    ClientSender(ProtocolSender& protocol, Queue<protocol::ClientMessage>& commands);

protected:
    void execute() override;
};

#endif
