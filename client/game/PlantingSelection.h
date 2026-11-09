#pragma once

#include <optional>

#include "game/CarrySelection.h"
#include "game/LocalWorldView.h"
#include "world/BuildPlaceCatalog.h"
#include "world/Grid.h"

namespace game {

// La selección de lugar: la celda marcada y el estado del marcador (el
// que corresponde según el terreno). DTO plano.
struct PlantingSpot {
	world::GridCell cell;
	world::BuildPlaceState state;
};

// Dónde se muestra el marcador de selección de lugar. Es presentación
// LOCAL (el mismo concepto que la selección del HUD): el marcador
// aparece sobre la celda que el PUNTERO apunta cuando el jugador
// sostiene un saco — "disponible" (verde) si es tierra tratada sin
// sembrar, "ocupado" (rojo) si ya tiene su cultivo. NO sigue al
// jugador: se muestra donde el cursor se apoya (hover). Cuando el
// servidor mande su selección, este cálculo se retira y el dibujo no
// cambia (contrato §6).
//
// Estilo ViewMotion/CarrySelection: se observa una vez por tick; las
// consultas no calculan nada.
class PlantingSelection {
public:
	// Recalcula dónde marcar. `carry` ya se observó este tick y
	// `hovered` es la celda del mundo bajo el puntero.
	void observe(const LocalWorldView& view, const CarrySelection& carry,
		     const world::GridCell& hovered);

	// La celda marcada con su estado, o nullopt si no hay nada que
	// marcar (sin saco en la mano o el puntero fuera de tierra tratada).
	const std::optional<PlantingSpot>& spot() const;

private:
	// ¿Un click sobre esta celda siembra EXACTAMENTE esa celda? El
	// mensaje de "usar" solo lleva dirección: llega a las 4 celdas
	// adyacentes en eje a la del jugador (la celda propia planta "al
	// frente", por eso no cuenta). Sin este gate el marcador invitaría
	// a clicks que no hacen nada.
	bool isReachable(const LocalWorldView& view,
			 const world::GridCell& hovered) const;

	std::optional<PlantingSpot> spot_;
};

}
