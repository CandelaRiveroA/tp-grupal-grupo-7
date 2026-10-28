#pragma once

#include <array>
#include <map>
#include <vector>

#include "world/CharacterAppearance.h"

namespace world {


enum class CreatureKind {
	Player,
	Chicken,
	Cow,
};

struct MoveCadence {
	int stepTicks;
	int frameTicks;
};

struct DirectionAppearance {
	const std::map<ActionKind, ActionData>* actions;
	bool mirrored;
};

struct CreatureData {
	const std::array<DirectionAppearance, 4>* directions;
	MoveCadence cadence;
};

}
