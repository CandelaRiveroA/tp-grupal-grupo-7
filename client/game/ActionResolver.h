#pragma once

#include <cstddef>
#include <map>
#include <optional>
#include <string>

#include "game/CarrySelection.h"
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

// Resuelve la pose efectiva de cada entidad: la acción que se anima, su
// fase y —para el jugador local— qué contenido sostiene. La caminata y
// el "sostener" son presentación local (selección del HUD); el resto de
// las acciones las decide la fuente de estado. El render solo anima lo
// que resuelve esta clase (contrato §6/§10).
class ActionResolver {
public:
	ActionResolver(const ViewMotion& motion, const CarrySelection& carry);

	world::ActionKind actionFor(const ViewEntity& entity,
				    std::size_t index, int it) const;

	int phaseFor(const ViewEntity& entity, std::size_t index, int it) const;

	// Contenido sostenido sobre la cabeza (el que dibuja la pose
	// Carry), o nullopt si no se sostiene nada.
	const std::optional<std::string>& heldContentId() const;

	// Con qué reloj anima una acción: la clasificación de la propia
	// tabla (la fuente única de "esto es cadencia de paso", que usan
	// los pasos del audio). Sumar una acción con piernas = una fila.
	PhaseClock clockFor(world::ActionKind action) const;

	// ¿La pose es de las que sostienen (las variantes de carry)? El
	// render lo usa para saber cuándo dibuja el ítem sobre la cabeza;
	// no decide nada: la respuesta sale de la tabla de variantes.
	bool isHoldingPose(world::ActionKind action) const;

private:
	// La acción que la fuente reporta, con el golpe genérico (UseTool)
	// traducido a la animación propia de la herramienta activa.
	world::ActionKind toolAction(const ViewEntity& entity) const;

	const ViewMotion& motion_;
	const CarrySelection& carry_;
	// Qué reloj anima cada acción: sumar una acción nueva = una entrada.
	std::map<world::ActionKind, PhaseClock> phaseClocks_;
	// Variante "sosteniendo" de cada acción del jugador local (Idle ->
	// Carry, Walk -> CarryWalk; lo no mapeado -> Carry mientras
	// sostenga). Sumar una variante = una fila + sus datos.
	std::map<world::ActionKind, world::ActionKind> carryVariants_;
	// Animación propia de cada herramienta cuando la fuente manda el
	// golpe genérico (UseTool): la regadera se anima como Water. Una
	// herramienta sin fila se anima como UseTool (el pico, la pala).
	// Sumar una herramienta con animación propia = una fila + sus datos.
	std::map<world::ItemKind, world::ActionKind> toolActions_;
};

}
