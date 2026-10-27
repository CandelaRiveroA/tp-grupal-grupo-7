#pragma once

#include <cstddef>

#include "game/LocalWorldView.h"
#include "game/ToolUse.h"
#include "game/ViewMotion.h"
#include "world/CharacterAppearance.h"

namespace game {

class ActionResolver {
public:
	ActionResolver(const ViewMotion& motion, const ToolUse& toolUse,
		       std::size_t toolUserIndex);

	world::ActionKind actionFor(const ViewEntity& entity,
				    std::size_t index, int it) const;

	int phaseFor(const ViewEntity& entity, std::size_t index, int it) const;

private:
	const ViewMotion& motion_;
	const ToolUse& toolUse_;
	std::size_t toolUserIndex_;
};

}
