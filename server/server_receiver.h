#ifndef SERVER_RECEIVER_H
#define SERVER_RECEIVER_H

#include <map>

#include "common_command.h"
#include "common_protocol_receiver.h"
#include "common_queue.h"
#include "common_worker.h"

class ServerReceiver: public Worker {
private:
    ProtocolReceiver& protocol;
    Queue<Command>& commands;
    using CommandHandler = void (ServerReceiver::*)(const Command&);
    std::map<CommandType, CommandHandler> handlers;

    void handle_move_command(const Command& command);

public:
    ServerReceiver(ProtocolReceiver& protocol, Queue<Command>& commands);

protected:
    void execute() override;
};

#endif
