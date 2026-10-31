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

	// Duración en ticks del golpe (UseTool): frames x ticks por cuadro de
	// la acción. Devuelve 1 como mínimo (y también si la criatura no
	// tiene la acción), para que la ventana del golpe nunca sea nula.
	int toolUseDurationTicks() const;
};

}
