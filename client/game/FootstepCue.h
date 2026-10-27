#pragma once

namespace game {

class FootstepCue {
public:
	bool update(bool moving, int walkFrame);

private:
	bool moving_ = false;
	int frame_ = 0;
};

}
