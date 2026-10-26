#include "game/FootstepCue.h"

namespace game {

bool FootstepCue::update(bool moving, int walkFrame) {
	if (!moving) {
		moving_ = false;
		return false;
	}

	if (!moving_) {
		moving_ = true;
		frame_ = walkFrame;
		return true;
	}

	const bool changed = (walkFrame != frame_);
	frame_ = walkFrame;
	return changed;
}

}
