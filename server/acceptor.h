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

class Acceptor: public Thread {
private:
    Socket listening;
    Queue<MatchEvent>& events;
    uint8_t next_player_id;
    std::vector<std::unique_ptr<ClientHandler>> handlers;

public:
    Acceptor(const char* service, Queue<MatchEvent>& events);

    void run() override;
    void stop() override;

    // Solo debe llamarse despues de join(): no hay otro hilo tocando handlers.
    void disconnect_clients();
};

#endif
