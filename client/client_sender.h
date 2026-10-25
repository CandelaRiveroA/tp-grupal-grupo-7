#ifndef CLIENT_SENDER_H
#define CLIENT_SENDER_H

#include <map>

#include "common_command.h"
#include "common_protocol_sender.h"
#include "common_queue.h"
#include "common_worker.h"

class ClientSender: public Worker {
private:
    ProtocolSender& protocol;
    Queue<Command>& commands;
    using CommandHandler = void (ClientSender::*)(const Command&);
    std::map<CommandType, CommandHandler> handlers;

    void handle_move_command(const Command& command);

public:
    ClientSender(ProtocolSender& protocol, Queue<Command>& commands);

protected:
    void execute() override;
};

#endif
