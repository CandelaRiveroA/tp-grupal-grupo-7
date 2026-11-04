#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <memory>
#include <vector>

#include "client_handler.h"
#include "common_socket.h"
#include "common_thread.h"
#include "matches_monitor.h"
#include "server_output_monitor.h"

class Acceptor: public Thread {
private:
    Socket listening;
    MatchesMonitor& matches;
    ServerOutputMonitor& output;
    std::vector<std::unique_ptr<ClientHandler>> handlers;

    void collectFinishedHandlers();
    void clear();

public:
    Acceptor(const char* service, MatchesMonitor& matches, ServerOutputMonitor& output);

    void run() override;
    void stop() override;
};

#endif
