#include "game/ActionResolver.h"

#include "game/GameConstants.h"

namespace game {

ActionResolver::ActionResolver(const ViewMotion& motion,
			       const CarrySelection& carry)
	: motion_(motion)
	, carry_(carry)
	, actionSpecs_{
		  // Locomoción: la caminata la deduce la presentación
		  // (relajo del paso) y las piernas van con el paso.
		  {world::ActionKind::Idle,
		   ActionSpec{ActionRole::Locomotion, PhaseClock::Action}},
		  {world::ActionKind::Walk,
		   ActionSpec{ActionRole::Locomotion, PhaseClock::Step}},
		  {world::ActionKind::CarryWalk,
		   ActionSpec{ActionRole::Locomotion, PhaseClock::Step}},
		  // Acción: la enciende y apaga la fuente; se anima tal
		  // cual (la caminata no la pisa) con el reloj de la acción.
		  {world::ActionKind::UseTool,
		   ActionSpec{ActionRole::Action, PhaseClock::Action}},
		  {world::ActionKind::Water,
		   ActionSpec{ActionRole::Action, PhaseClock::Action}},
		  {world::ActionKind::Milk,
		   ActionSpec{ActionRole::Action, PhaseClock::Action}},
		  {world::ActionKind::Pickup,
		   ActionSpec{ActionRole::Action, PhaseClock::Action}},
		  {world::ActionKind::Carry,
		   ActionSpec{ActionRole::Action, PhaseClock::Action}},
		  {world::ActionKind::Attack,
		   ActionSpec{ActionRole::Action, PhaseClock::Action}},
		  // Daño recibido: DOMINANTE (parpadea aunque camine o
		  // sostenga algo). Su pose en el jugador es la titilación
		  // (frames vacíos alternados, characters.toml).
		  {world::ActionKind::Damage,
		   ActionSpec{ActionRole::Action, PhaseClock::Action, true}},
		  {world::ActionKind::Death,
		   ActionSpec{ActionRole::Action, PhaseClock::Action}},
		  {world::ActionKind::Faint,
		   ActionSpec{ActionRole::Action, PhaseClock::Action, true}},
		  {world::ActionKind::Sword,
		   ActionSpec{ActionRole::Action, PhaseClock::Action}},
		  {world::ActionKind::Revive,
		   ActionSpec{ActionRole::Action, PhaseClock::Action}},
	  }
	, carryVariants_{
		  {world::ActionKind::Idle, world::ActionKind::Carry},
		  {world::ActionKind::Walk, world::ActionKind::CarryWalk},
	  }
	, toolActions_{
		  {world::ItemKind::WateringCan, world::ActionKind::Water},
		  {world::ItemKind::Sword, world::ActionKind::Sword},
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
	// Las acciones DOMINANTES (Faint, Damage) mandan sobre la caminata
	// y el carry: el personaje puede seguir moviéndose (no se bloquea
	// el input), pero la pose es la de la acción hasta que la fuente la
	// apague. Lo decide la FILA de actionSpecs_ (dato), no un if por
	// acción.
	if (specFor(entity.action).dominant) {
		return entity.action;
	}
	// La caminata es presentación local (la interpolación del paso) y
	// solo pisa a las acciones de LOCOMOCIÓN. Las que enciende la
	// fuente (golpe, daño, muerte, ...) se animan tal cual aunque la
	// entidad se esté moviendo: la fuente manda la acción, el cliente
	// la anima (contrato §10).
	const world::ActionKind fromSource = toolAction(entity);
	world::ActionKind base = fromSource;
	if (specFor(fromSource).role == ActionRole::Locomotion) {
		base = motion_.isWalking(index, it, entity.moving)
			       ? world::ActionKind::Walk
			       : fromSource;
	}
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
	return specFor(action).clock;
}

const ActionSpec& ActionResolver::specFor(const world::ActionKind action) const {
	return actionSpecs_.at(action);
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
