#pragma once

#include "world/CharacterAppearance.h"

namespace game {

// Ventana de ACCIÓN LOCAL del jugador: dispara una acción de la fuente
// durante N ticks de presentación y después la suelta (la vista vuelve a
// mandar). Es la presentación local de las demos de input (el daño
// recibido): ANDAMIAJE TEMPORAL, muere cuando el servidor mande la
// acción. La ventana se construye de los MISMOS datos de la acción
// (ActionData, contrato §10): open(action, ticks) recibe los ticks que
// ya calculó el dueño de la criatura.
class LocalActionWindow {
public:
	// Abre la ventana sobre una acción, por los ticks recibidos (>= 1).
	void open(world::ActionKind action, int ticks);

	// Corre un tick de la ventana; al llegar a 0 la cierra.
	void advance();

	// true mientras la ventana esté abierta.
	bool active() const;

	// La acción que manda mientras la ventana esté abierta.
	world::ActionKind action() const;

private:
	world::ActionKind action_ = world::ActionKind::Idle;
	int ticksRemaining_ = 0;
};

}  // namespace game
