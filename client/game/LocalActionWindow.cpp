#include "game/LocalActionWindow.h"

namespace game {

void LocalActionWindow::open(const world::ActionKind action, const int ticks) {
	action_ = action;
	ticksRemaining_ = ticks > 0 ? ticks : 1;
}

void LocalActionWindow::advance() {
	if (ticksRemaining_ > 0) {
		--ticksRemaining_;
	}
}

bool LocalActionWindow::active() const { return ticksRemaining_ > 0; }

world::ActionKind LocalActionWindow::action() const { return action_; }

}  // namespace game
