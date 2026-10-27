#pragma once

#include "common/common_command.h"
#include "game/LocalWorldView.h"

namespace game {

class GameClient {
public:
	virtual ~GameClient() = default;

	virtual void sendCommand(const protocol::Command& command) = 0;

	virtual const LocalWorldView& worldState() const = 0;
};

}
