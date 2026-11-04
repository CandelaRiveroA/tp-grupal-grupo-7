#include "game/SoundGate.h"

#include <algorithm>
#include <utility>

namespace game {

SoundGate::SoundGate(std::vector<std::string> roomCues)
	: roomCues_(std::move(roomCues)) {}

bool SoundGate::allows(CueKind cue) const {
	const char* name = cues_.nameOf(cue);
	if (*name == '\0') {
		// Cue sin fila en el vocabulario: ninguna sala lo declara.
		return false;
	}
	return std::find(roomCues_.begin(), roomCues_.end(), name) !=
	       roomCues_.end();
}

}
