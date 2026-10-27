#ifndef MATCH_H
#define MATCH_H

#include <cstdint>
#include <map>
#include <memory>
#include <variant>

#include "domain/map/world.h"

#include "common_command.h"
#include "common_queue.h"
#include "common_server_status.h"
#include "common_worker.h"
#include "server_output_monitor.h"

struct PlayerMessage {
    uint8_t player_id;
    protocol::ClientMessage message;
};

struct PlayerJoined {
    uint8_t player_id;
    std::shared_ptr<Queue<ServerStatus>> statuses;
};

using MatchEvent = std::variant<PlayerJoined, PlayerMessage>;

class Match: public Worker {
private:
    Queue<MatchEvent>& events;
    ServerOutputMonitor& output;
    World world;
    std::map<uint8_t, std::shared_ptr<Queue<ServerStatus>>> outputs;

    void handle(const PlayerJoined& joined);
    void handle(const PlayerMessage& event);
    void apply(uint8_t player_id, const protocol::MoveMessage& message);
    void publish(uint8_t player_id);

public:
    Match(Queue<MatchEvent>& events, ServerOutputMonitor& output);

protected:
    void execute() override;
};

#endif
