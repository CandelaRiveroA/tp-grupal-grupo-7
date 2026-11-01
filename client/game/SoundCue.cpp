#include "game/SoundCue.h"

#include <utility>

namespace game {

SoundCueCatalog::SoundCueCatalog() {
	names_.push_back({CueKind::ChickenCluck, "chicken_cluck"});
	names_.push_back({CueKind::CowMoo, "cow_moo"});
	emitters_.push_back({world::CreatureKind::Chicken,
			     CueKind::ChickenCluck});
	emitters_.push_back({world::CreatureKind::Cow, CueKind::CowMoo});
}

const char* SoundCueCatalog::nameOf(CueKind cue) const {
	for (const auto& [mapsCue, name] : names_) {
		if (mapsCue == cue) {
			return name;
		}
	}
	// Cue sin fila: ninguna sala lo puede declarar.
	return "";
}

std::optional<CueKind> SoundCueCatalog::cueOf(
	world::CreatureKind kind) const {
	for (const auto& [emitter, cue] : emitters_) {
		if (emitter == kind) {
			return cue;
		}
	}
	return std::nullopt;
}

}
