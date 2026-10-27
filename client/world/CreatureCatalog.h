#pragma once

#include <array>
#include <map>
#include <vector>

#include "world/CharacterAppearance.h"

namespace world {


enum class CreatureKind {
	Player,
};

struct MoveCadence {
	int stepTicks;
	int frameTicks;
};

struct DirectionAppearance {
	std::map<ActionKind, ActionData> actions;
	bool mirrored = false;
};

struct CreatureData {
	std::array<DirectionAppearance, 4> directions;
	MoveCadence cadence{0, 0};
};

}
