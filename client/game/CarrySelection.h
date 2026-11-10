#pragma once

#include <optional>
#include <string>

#include "game/LocalWorldView.h"
#include "world/ContentTraits.h"

namespace game {

// Qué contenido sostiene el jugador local sobre la cabeza, derivado de
// su selección en el inventario: si el slot activo tiene un contenido
// SOLTABLE (rasgo `droppable` de world::ContentTraits), ese es el que se
// sostiene; herramientas, slot vacío o contenido no soltable no
// sostienen nada. Es presentación LOCAL de la selección (el mismo
// concepto que el marco rojo del HUD), no estado del servidor: cuando
// el servidor mande qué se sostiene, este cálculo se retira y el dibujo
// no cambia (contrato §6).
//
// Estilo ViewMotion: se observa una vez por tick con el estado y la
// selección activa; las consultas no calculan nada (convención §9:
// datos frescos por tick, sin recalcular por dibujo).
class CarrySelection {
public:
	explicit CarrySelection(const world::ContentTraits& traits);

	// Recalcula qué sostiene el jugador local. activeSlot = slot
	// activo del inventario (el que muestra el marco rojo del HUD).
	void observe(const LocalWorldView& view, int activeSlot);

	// El contentId sostenido, o nullopt si no sostiene nada.
	const std::optional<std::string>& held() const;

private:
	const world::ContentTraits& traits_;
	std::optional<std::string> held_;
};

}
