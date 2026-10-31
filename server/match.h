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
#include "world_view.h"

struct PlayerMessage {
    uint8_t player_id;
    protocol::ClientMessage message;
};

struct PlayerJoined {
    uint8_t player_id;
    std::shared_ptr<Queue<ServerMessage>> statuses;
};

struct PlayerLeft {
    uint8_t player_id;
};

using MatchEvent = std::variant<PlayerJoined, PlayerMessage, PlayerLeft>;

class Match: public Worker {
private:
    Queue<MatchEvent>& events;
    ServerOutputMonitor& output;
    World world;
    std::map<uint8_t, std::shared_ptr<Queue<ServerMessage>>> outputs;
    WorldView views;

    void handle(const PlayerJoined& joined);
    void handle(const PlayerMessage& event);
    void handle(const PlayerLeft& left);
    void sync_all();
    void place_minerals();
    void broadcast(const ServerMessage& message);
    void send_to(uint8_t player_id, const ServerMessage& message);
    void closeOutputs();

public:
    Match(Queue<MatchEvent>& events, ServerOutputMonitor& output);

protected:
    void execute() override;
};

#endif
