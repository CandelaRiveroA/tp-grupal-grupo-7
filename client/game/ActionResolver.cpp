#include "game/ActionResolver.h"

namespace game {

ActionResolver::ActionResolver(const ViewMotion& motion)
	: motion_(motion)
	, phaseClocks_{
		  {world::ActionKind::Idle, PhaseClock::Action},
		  {world::ActionKind::Walk, PhaseClock::Step},
		  {world::ActionKind::UseTool, PhaseClock::Action},
		  {world::ActionKind::Milk, PhaseClock::Action},
		  {world::ActionKind::Pickup, PhaseClock::Action},
	  } {}

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
	const world::ActionKind action = actionFor(entity, index, it);
	if (phaseClocks_.at(action) == PhaseClock::Step) {
		return motion_.animationTickFor(index, it);
	}
	return motion_.actionTickFor(index, it);
}

}
