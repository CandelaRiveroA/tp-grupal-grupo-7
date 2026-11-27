#pragma once

#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "game/CarrySelection.h"
#include "game/LocalWorldView.h"
#include "world/FurnitureGeometry.h"
#include "world/Grid.h"

namespace game {

// Dónde se colocaría el mueble sostenido: su base, las celdas de piso
// que ocuparía y si ese lugar está libre. DTO plano.
struct PlacementSpot {
	std::string contentId;
	world::GridCell base;
	// La celda que se le manda a la fuente al colocar (la base corrida
	// por el ancla del mueble; en un bloque es la misma base).
	world::GridCell source;
	// Hacia dónde mira (la elige el jugador con el click derecho).
	world::FurnitureFacing facing;
	// Lado elegido: del jugador a la celda base (dx, dy de una celda).
	world::GridCell toward;
	std::vector<world::GridCell> cells;
	bool valid;
};

// Decide dónde mostrar la vista previa de colocación del mueble que el
// jugador sostiene: sobre la celda que apunta el puntero, si está al
// alcance (las 4 vecinas en eje, como plantar). La validez es solo una
// PISTA visual (celdas dentro del mapa y libres según la vista): la
// fuente de estado decide de verdad. Presentación LOCAL, estilo
// PlantingSelection: se observa una vez por tick.
class PlacementSelection {
public:
	explicit PlacementSelection(const world::FurnitureGeometry& furniture);

	void observe(const LocalWorldView& view, const CarrySelection& carry,
		     const world::GridCell& hovered,
		     const world::GridSpan& sceneGrid,
		     world::FurnitureFacing facing);

	// nullopt si no hay mueble en la mano o el puntero está fuera de alcance.
	const std::optional<PlacementSpot>& spot() const;

private:
	using CellKey = std::pair<int, int>;

	bool isReachable(const LocalWorldView& view,
			 const world::GridCell& hovered) const;

	// Celdas que la vista ya ocupa (entidades, recursos, ítems, cultivos
	// y la huella de cada mueble colocado).
	std::set<CellKey> occupiedCells(const LocalWorldView& view) const;

	bool isFree(const std::vector<world::GridCell>& cells,
		    const std::set<CellKey>& occupied,
		    const world::GridSpan& sceneGrid) const;

	const world::FurnitureGeometry& furniture_;
	std::optional<PlacementSpot> spot_;
};

}
