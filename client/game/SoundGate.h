#pragma once

#include <string>
#include <vector>

#include "game/SoundCue.h"

namespace game {

class SoundGate {
public:
	explicit SoundGate(std::vector<std::string> roomCues);

	bool allows(CueKind cue) const;

private:
	SoundCueCatalog cues_;

	std::vector<std::string> roomCues_;
};

}
