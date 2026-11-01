#ifndef SERVER_H
#define SERVER_H

#include "acceptor.h"
#include "matches_monitor.h"
#include "server_output_monitor.h"

class Server {
private:
    ServerOutputMonitor output;
    MatchesMonitor matches;
    Acceptor acceptor;

public:
    explicit Server(const char* service);

    void run();
};

#endif
