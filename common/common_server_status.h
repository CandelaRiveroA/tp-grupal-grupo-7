#ifndef COMMON_SERVER_STATUS_H
#define COMMON_SERVER_STATUS_H

#include <cstdint>

#include "common_command_types.h"

enum class ServerMessageType : uint8_t { Status = 1 };

struct ServerStatus {
    PlayerId player_id;
    uint8_t x;
    uint8_t y;
    Direction direction;

    ServerStatus(PlayerId player_id, uint8_t x, uint8_t y, Direction direction):
            player_id(player_id), x(x), y(y), direction(direction) {}
};

#endif
