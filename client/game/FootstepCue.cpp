#include "game/FootstepCue.h"

namespace game {

bool FootstepCue::update(bool moving, int stepFrame) {
	if (!moving) {
		moving_ = false;
		return false;
	}

	if (!moving_) {
		moving_ = true;
		frame_ = stepFrame;
		return true;
	}

	const bool changed = (stepFrame != frame_);
	frame_ = stepFrame;
	return changed;
}

}
