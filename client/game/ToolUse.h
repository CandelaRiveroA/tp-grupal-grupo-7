#pragma once

#include "world/CharacterAppearance.h"

namespace game {

class ToolUse {
public:
	explicit ToolUse(int durationTicks);

	explicit ToolUse(const world::ActionData& action);

	void trigger(int it);

	bool active(int it) const;

	int elapsedTicks(int it) const;

private:
	int durationTicks_;
	bool started_ = false;
	int startIt_ = 0;
};

}
