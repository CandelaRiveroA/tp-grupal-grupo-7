#ifndef COMMON_SERVER_STATUS_H
#define COMMON_SERVER_STATUS_H

#include <cstdint>
#include <variant>

#include "common_constants.h"

enum class ServerMessageType : uint8_t { Status = 1, Welcome = 2, Leave = 3 };

struct ServerStatus {
    uint8_t player_id;
    uint16_t x;
    uint16_t y;
    Direction direction;

    ServerStatus(uint8_t player_id, uint16_t x, uint16_t y, Direction direction):
            player_id(player_id), x(x), y(y), direction(direction) {}
};

struct ServerWelcome {
    uint8_t player_id;
};

struct ServerLeave {
    uint8_t player_id;
};

using ServerMessage = std::variant<ServerStatus, ServerLeave>;

#endif
