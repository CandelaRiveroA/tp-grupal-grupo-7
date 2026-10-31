#pragma once

#include "world/CharacterAppearance.h"
#include "world/CreatureCatalog.h"

namespace world {

struct EntityAppearance {
	const Pose* pose;
	bool mirrored;
};

// Resuelve la apariencia de una criatura a partir de sus datos: dado el
// facing, la acción y el tick de animación, decide qué cuadro y qué pose
// corresponden. Dueño del concepto "cómo se ve esta criatura ahora".
// Separa la lógica de animación de CreatureData (datos puros).
class AppearanceResolver {
public:
	explicit AppearanceResolver(const CreatureData& creature);

	int frameIndexFor(Direction facing, ActionKind action,
			  int animationTick) const;

	EntityAppearance appearanceFor(Direction facing, ActionKind action,
				       int animationTick) const;

private:
	const ActionData& actionData(Direction facing,
				     ActionKind action) const;

	const CreatureData& creature_;
};

}
