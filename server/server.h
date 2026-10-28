#ifndef SERVER_H
#define SERVER_H

#include "acceptor.h"
#include "common_queue.h"
#include "match.h"
#include "server_output_monitor.h"

class Server {
private:
    Queue<MatchEvent> events;
    ServerOutputMonitor output;
    Match match;
    Acceptor acceptor;

public:
    explicit Server(const char* service);

    void run();
};

#endif
