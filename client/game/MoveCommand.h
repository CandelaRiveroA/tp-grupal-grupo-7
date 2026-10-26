#pragma once

#include "common/common_command.h"
#include "game/LocalWorldView.h"

namespace game {

class MoveCommand {
public:
	protocol::Command propose(const LocalWorldView& state,
				  protocol::Direction direction) const;

	protocol::Command hold(const LocalWorldView& state,
			       protocol::Direction direction) const;

	protocol::Command use(const LocalWorldView& state,
			      protocol::Direction direction) const;

private:
	protocol::Command walkTo(const LocalWorldView& state, int offsetX,
				 int offsetY,
				 protocol::Direction direction) const;
};

}
