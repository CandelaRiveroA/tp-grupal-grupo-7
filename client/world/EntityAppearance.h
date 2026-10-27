#pragma once

#include "world/CharacterAppearance.h"
#include "world/CreatureCatalog.h"

namespace world {

struct EntityAppearance {
	const Pose* pose;
	bool mirrored;
};

int frameIndexFor(const CreatureData& creature, Direction facing,
		  ActionKind action, int animationTick);

EntityAppearance appearanceFor(const CreatureData& creature,
			       Direction facing, ActionKind action,
			       int animationTick);

}
