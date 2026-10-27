#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <cstdint>
#include <memory>

#include "common_protocol_receiver.h"
#include "common_protocol_sender.h"
#include "common_queue.h"
#include "common_server_status.h"
#include "common_socket.h"
#include "common_thread.h"
#include "match.h"
#include "server_receiver.h"
#include "server_sender.h"

class ClientHandler: public Thread {
private:
    Socket peer;
    uint8_t player_id;
    Queue<MatchEvent>& events;
    std::shared_ptr<Queue<ServerStatus>> statuses;
    ProtocolSender protocol_sender;
    ProtocolReceiver protocol_receiver;
    ServerReceiver receiver;
    ServerSender sender;

public:
    ClientHandler(Socket&& client_socket, uint8_t player_id, Queue<MatchEvent>& events);

    void run() override;
    void stop() override;
};

#endif
