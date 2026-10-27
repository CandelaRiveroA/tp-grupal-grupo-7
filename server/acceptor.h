#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <cstdint>
#include <memory>
#include <vector>

#include "client_handler.h"
#include "common_queue.h"
#include "common_socket.h"
#include "common_thread.h"
#include "match.h"
#include "server_output_monitor.h"

class Acceptor: public Thread {
private:
    Socket listening;
    Queue<MatchEvent>& events;
    ServerOutputMonitor& output;
    uint8_t next_player_id;
    std::vector<std::unique_ptr<ClientHandler>> handlers;

    void collectFinishedHandlers();
    void clear();

public:
    Acceptor(const char* service, Queue<MatchEvent>& events, ServerOutputMonitor& output);

    void run() override;
    void stop() override;
};

#endif
