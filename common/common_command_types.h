#ifndef COMMON_COMMAND_TYPES_H
#define COMMON_COMMAND_TYPES_H

#include <cstdint>

#define MAX_PLAYER_ID 4

using PlayerId = uint8_t;

enum class CommandType : uint8_t { Move = 1 };

enum class Direction : uint8_t { Up = 0, Down = 1, Left = 2, Right = 3 };

#endif
