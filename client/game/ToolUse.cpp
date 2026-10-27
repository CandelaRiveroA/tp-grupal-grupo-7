#include "game/ToolUse.h"

namespace game {

ToolUse::ToolUse(int durationTicks) : durationTicks_(durationTicks) {}

ToolUse::ToolUse(const world::ActionData& action) {
	const int duration =
		static_cast<int>(action.frames.size()) * action.frameTicks;
	durationTicks_ = duration > 0 ? duration : 1;
}

void ToolUse::trigger(int it) {
	started_ = true;
	startIt_ = it;
}

bool ToolUse::active(int it) const {
	if (!started_) {
		return false;
	}
	const int elapsed = it - startIt_;
	return elapsed >= 0 && elapsed < durationTicks_;
}

int ToolUse::elapsedTicks(int it) const {
	if (!started_) {
		return 0;
	}
	const int elapsed = it - startIt_;
	return elapsed > 0 ? elapsed : 0;
}

}
