#ifndef MATCH_H
#define MATCH_H

#include "common_command.h"
#include "common_queue.h"
#include "common_server_status.h"
#include "common_worker.h"

class Match: public Worker {
private:
    Queue<Command>& commands;
    Queue<ServerStatus>& statuses;

public:
    Match(Queue<Command>& commands, Queue<ServerStatus>& statuses);

protected:
    void execute() override;
};

#endif
