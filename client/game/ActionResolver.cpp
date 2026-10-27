#include "game/ActionResolver.h"

namespace game {

ActionResolver::ActionResolver(const ViewMotion& motion,
			       const ToolUse& toolUse,
			       std::size_t toolUserIndex)
	: motion_(motion)
	, toolUse_(toolUse)
	, toolUserIndex_(toolUserIndex) {}

world::ActionKind ActionResolver::actionFor(const ViewEntity& entity,
					    std::size_t index, int it) const {
	if (motion_.isWalking(index, it, entity.moving)) {
		return world::ActionKind::Walk;
	}
	if (toolUse_.active(it) && index == toolUserIndex_) {
		return world::ActionKind::UseTool;
	}
	return entity.action;
}

int ActionResolver::phaseFor(const ViewEntity& entity, std::size_t index,
			     int it) const {
	if (actionFor(entity, index, it) == world::ActionKind::UseTool) {
		return toolUse_.elapsedTicks(it);
	}
	return motion_.animationTickFor(index, it);
}

}
