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
	Orc,
	Skeleton,
	Vampire,
};

struct MoveCadence {
	int stepTicks;
	int frameTicks;
};

// Presentación del contenido sostenido sobre la cabeza (pose Carry):
// qué tan arriba de la cabeza se dibuja y a qué escala. Dato de
// presentación por criatura (characters.toml [characters.<id>.hold]).
struct HoldPresentation {
	int offsetY = 0;
	double scale = 1.0;
};

struct DirectionAppearance {
	std::map<ActionKind, ActionData> actions;
	bool mirrored = false;
};

struct CreatureData {
	std::array<DirectionAppearance, 4> directions;
	MoveCadence cadence{0, 0};

	// Cómo se dibuja el contenido sostenido sobre la cabeza (pose
	// Carry) en esta criatura.
	HoldPresentation hold;

	// Duración en ticks de la VENTANA de una acción (su ciclo entero:
	// frames x frameTicks), o 1 como mínimo (también si la criatura no
	// tiene la acción): la ventana nunca es nula. Es la MISMA cuenta
	// que usa la animación: ventana y animación no pueden desfasarse
	// (contrato §10).
	int actionWindowTicks(ActionKind action) const;

	// Ventana de la acción de usar herramienta (la que espera la
	// sesión de red para animar el golpe). Mismo cálculo que
	// actionWindowTicks(UseTool).
	int toolUseDurationTicks() const;

	// Duración de UN ciclo de la acción (frames x frameTicks), o 0 si la
	// criatura no la tiene.
	int actionCycleTicks(ActionKind action) const;
};

}
