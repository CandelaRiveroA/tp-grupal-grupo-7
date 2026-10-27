#ifndef SERVER_RECEIVER_H
#define SERVER_RECEIVER_H

#include <cstdint>

#include "common_protocol_receiver.h"
#include "common_worker.h"
#include "match.h"

class ServerReceiver: public Worker {
private:
    ProtocolReceiver& protocol;
    Queue<MatchEvent>& events;
    uint8_t player_id;

public:
    ServerReceiver(ProtocolReceiver& protocol, Queue<MatchEvent>& events, uint8_t player_id);

protected:
    void execute() override;
};

#endif
