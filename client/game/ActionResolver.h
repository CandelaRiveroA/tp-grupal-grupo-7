#pragma once

#include <cstddef>

#include "game/LocalWorldView.h"
#include "game/ViewMotion.h"
#include "world/CharacterAppearance.h"

namespace game {

class ActionResolver {
public:
	explicit ActionResolver(const ViewMotion& motion);

	world::ActionKind actionFor(const ViewEntity& entity,
				    std::size_t index, int it) const;

	int phaseFor(const ViewEntity& entity, std::size_t index, int it) const;

private:
	const ViewMotion& motion_;
};

}
