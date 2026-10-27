#include "game/MoveCommand.h"

#include <algorithm>
#include <cstdint>

namespace game {

protocol::Command MoveCommand::propose(const LocalWorldView& state,
				       protocol::Direction direction) const {
	int offsetX = 0;
	int offsetY = 0;
	switch (direction) {
	case protocol::Direction::Left:
		offsetX -= 1;
		break;
	case protocol::Direction::Right:
		offsetX += 1;
		break;
	case protocol::Direction::Back:
		offsetY -= 1;
		break;
	case protocol::Direction::Front:
		offsetY += 1;
		break;
	}
	return walkTo(state, offsetX, offsetY, direction);
}

protocol::Command MoveCommand::hold(const LocalWorldView& state,
				    protocol::Direction direction) const {
	return walkTo(state, 0, 0, direction);
}

protocol::Command MoveCommand::use(const LocalWorldView& state,
				   protocol::Direction direction) const {
	// Picar apunta a la celda que el jugador tiene ENFRENTE (según la
	// dirección que mira), no a la propia. Reusa el mismo offset por
	// dirección que propose(), pero con move = Pick.
	protocol::Command command = propose(state, direction);
	command.move = protocol::Move::Pick;
	return command;
}

protocol::Command MoveCommand::walkTo(const LocalWorldView& state,
				      int offsetX, int offsetY,
				      protocol::Direction direction) const {
	int currentX = 0;
	int currentY = 0;
	if (!state.entities.empty()) {
		currentX = state.entities.front().cellX;
		currentY = state.entities.front().cellY;
	}

	protocol::Command command;
	command.move = protocol::Move::Walk;
	command.x = static_cast<std::uint8_t>(std::max(0, currentX + offsetX));
	command.y = static_cast<std::uint8_t>(std::max(0, currentY + offsetY));
	command.direction = direction;
	return command;
}

}
