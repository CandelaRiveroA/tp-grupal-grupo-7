#ifndef SERVER_H
#define SERVER_H

#include "acceptor.h"
#include "common_queue.h"
#include "match.h"

class Server {
private:
    Queue<MatchEvent> events;
    Match match;
    Acceptor acceptor;

public:
    explicit Server(const char* service);

    void run();
};

#endif
