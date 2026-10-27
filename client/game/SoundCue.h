#pragma once

#include <optional>
#include <utility>
#include <vector>

#include "world/CreatureCatalog.h"

namespace game {

enum class CueKind {
	ChickenCluck,
};

struct ViewSoundEvent {
	int emitterId;
	CueKind cue;
};

// Dueña del vocabulario de cues: cómo se llama cada cue en los TOMLs y
// qué criatura lo emite
class SoundCueCatalog {
public:
	SoundCueCatalog();

	// Nombre del cue en los TOMLs. Un cue sin fila devuelve ""
	const char* nameOf(CueKind cue) const;

	// Qué cue emite este kind de criatura, o nullopt si no emite
	std::optional<CueKind> cueOf(world::CreatureKind kind) const;

private:
	std::vector<std::pair<CueKind, const char*>> names_;

	std::vector<std::pair<world::CreatureKind, CueKind>> emitters_;
};

}
