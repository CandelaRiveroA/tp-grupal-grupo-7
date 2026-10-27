#include "game/ActionResolver.h"

namespace game {

ActionResolver::ActionResolver(const ViewMotion& motion) : motion_(motion) {}

world::ActionKind ActionResolver::actionFor(const ViewEntity& entity,
					    std::size_t index, int it) const {
	if (motion_.isWalking(index, it, entity.moving)) {
		return world::ActionKind::Walk;
	}
	// El resto lo decide la fuente de estado (mock/servidor): si dice
	// UseTool, se anima el golpe; cuando vuelve a Idle, el cuerpo vuelve
	// a estar quieto. El cliente no decide la duración del golpe.
	return entity.action;
}

int ActionResolver::phaseFor(const ViewEntity& entity, std::size_t index,
			     int it) const {
	if (actionFor(entity, index, it) == world::ActionKind::UseTool) {
		return motion_.toolTickFor(index, it);
	}
	return motion_.animationTickFor(index, it);
}

}
