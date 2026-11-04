#include "game/ActionResolver.h"

#include "game/GameConstants.h"

namespace game {

ActionResolver::ActionResolver(const ViewMotion& motion,
			       const CarrySelection& carry)
	: motion_(motion)
	, carry_(carry)
	, phaseClocks_{
		  {world::ActionKind::Idle, PhaseClock::Action},
		  {world::ActionKind::Walk, PhaseClock::Step},
		  {world::ActionKind::UseTool, PhaseClock::Action},
		  {world::ActionKind::Water, PhaseClock::Action},
		  {world::ActionKind::Milk, PhaseClock::Action},
		  {world::ActionKind::Pickup, PhaseClock::Action},
		  {world::ActionKind::Carry, PhaseClock::Action},
		  {world::ActionKind::CarryWalk, PhaseClock::Step},
	  }
	, carryVariants_{
		  {world::ActionKind::Idle, world::ActionKind::Carry},
		  {world::ActionKind::Walk, world::ActionKind::CarryWalk},
	  }
	, toolActions_{
		  {world::ItemKind::WateringCan, world::ActionKind::Water},
	  } {}

world::ActionKind ActionResolver::toolAction(const ViewEntity& entity) const {
	// Solo el golpe genérico se re-interpreta según la herramienta: si
	// la fuente ya manda una acción concreta (Water, Milk, ...), se
	// respeta tal cual.
	if (entity.action != world::ActionKind::UseTool ||
	    !entity.activeTool.has_value()) {
		return entity.action;
	}
	const auto mapped = toolActions_.find(*entity.activeTool);
	return mapped != toolActions_.end() ? mapped->second : entity.action;
}

world::ActionKind ActionResolver::actionFor(const ViewEntity& entity,
					    std::size_t index, int it) const {
	// La caminata es presentación local (la interpolación del paso).
	const world::ActionKind base = motion_.isWalking(index, it, entity.moving)
					       ? world::ActionKind::Walk
					       : toolAction(entity);
	// Sostener un contenido soltable es presentación LOCAL de la
	// selección del HUD (como el marco rojo): mientras el jugador
	// local sostenga algo, su pose es SIEMPRE la variante "sosteniendo"
	// (Idle -> Carry, Walk -> CarryWalk y lo no mapeado -> Carry). La
	// fuente puede mandar cualquier acción (un golpe, ...): con el
	// saco en la cabeza solo se ven los brazos arriba y el contenido
	// sobre la cabeza.
	if (index == kLocalPlayerIndex && carry_.held().has_value()) {
		const auto variant = carryVariants_.find(base);
		return variant != carryVariants_.end() ? variant->second
						       : world::ActionKind::Carry;
	}
	// El resto lo decide la fuente de estado (mock/servidor): si dice
	// UseTool, se anima el golpe; cuando vuelve a Idle, el cuerpo vuelve
	// a estar quieto. El cliente no decide la duración del golpe.
	return base;
}

int ActionResolver::phaseFor(const ViewEntity& entity, std::size_t index,
			     int it) const {
	const world::ActionKind action = actionFor(entity, index, it);
	if (clockFor(action) == PhaseClock::Step) {
		return motion_.animationTickFor(index, it);
	}
	return motion_.actionTickFor(index, it);
}

const std::optional<std::string>& ActionResolver::heldContentId() const {
	return carry_.held();
}

PhaseClock ActionResolver::clockFor(const world::ActionKind action) const {
	return phaseClocks_.at(action);
}

bool ActionResolver::isHoldingPose(const world::ActionKind action) const {
	for (const auto& [base, variant] : carryVariants_) {
		(void)base;
		if (variant == action) {
			return true;
		}
	}
	return false;
}

}
