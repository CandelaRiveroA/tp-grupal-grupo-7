#ifndef COMMON_COMMAND_H
#define COMMON_COMMAND_H

#include <cstdint>

#include "common_game_direction.h"

namespace protocol {

enum class Move {
	Walk,
	Pick,
};

struct Command {
	Move move;
	std::uint8_t x;
	std::uint8_t y;
	Direction direction;
};

}

#endif
