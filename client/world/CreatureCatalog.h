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

	// Duración de UN ciclo de la acción (frames x frameTicks), o 0 si la
	// criatura no la tiene. Es la MISMA cuenta que usa la animación:
	// ventana y animación no pueden desfasarse (contrato §10).
	int actionCycleTicks(ActionKind action) const;
};

}
