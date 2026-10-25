#ifndef COMMON_COMMAND_H
#define COMMON_COMMAND_H

#include <cstdint>

#include "common_command_types.h"

struct Command {
    PlayerId player_id;
    CommandType type;
    uint8_t x;
    uint8_t y;
    Direction direction;

    Command(PlayerId player_id, CommandType type, uint8_t x, uint8_t y, Direction direction):
            player_id(player_id), type(type), x(x), y(y), direction(direction) {}
};

#endif
