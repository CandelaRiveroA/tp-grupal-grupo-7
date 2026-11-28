#pragma once

#include <vector>

#include "world/Grid.h"

namespace world {

// Geometría pura de la HUELLA de un mueble: las celdas de PISO que
// ocupa (lo que bloquea), no la altura del sprite (eso es
// FurnitureLayout). La celda base es abajo-izquierda: la huella crece
// hacia la derecha y hacia arriba, como el sprite.
class FurnitureFootprint {
public:
	explicit FurnitureFootprint(const GridSpan& span);

	// Las celdas de piso que ocupa al estar con su base en `base`.
	std::vector<GridCell> cellsAt(const GridCell& base) const;

private:
	GridSpan span_;
};

}
