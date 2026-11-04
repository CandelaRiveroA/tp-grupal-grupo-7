#pragma once

#include <cstddef>
#include <map>

#include "game/LocalWorldView.h"
#include "game/ViewMotion.h"
#include "world/CharacterAppearance.h"

namespace game {

// Qué reloj anima cada acción (convención §4: tabla, no switch): la
// cadencia del PASO para caminar, el reloj de la ACCIÓN para las que la
// fuente enciende y apaga (golpe, ordeñe, recogida, ...).
enum class PhaseClock {
	Step,
	Action,
};

class ActionResolver {
public:
	explicit ActionResolver(const ViewMotion& motion);

	world::ActionKind actionFor(const ViewEntity& entity,
				    std::size_t index, int it) const;

	int phaseFor(const ViewEntity& entity, std::size_t index, int it) const;

private:
	const ViewMotion& motion_;
	// Reloj por acción: sumar una acción nueva = una entrada.
	std::map<world::ActionKind, PhaseClock> phaseClocks_;
};

}
