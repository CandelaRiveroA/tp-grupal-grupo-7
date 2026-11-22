#pragma once

#include "world/Grid.h"

namespace game {

// ¿Está una celda sobre un rectángulo de celdas o en el anillo de
// `ring` celdas alrededor? Es la regla de cercanía de lo que se toca
// (la cama, un edificio): el click solo hace algo si el jugador está
// cerca, según la posición que manda la fuente de estado.
class AreaProximity {
public:
	explicit AreaProximity(int ring);

	bool around(const world::GridCell& at, const world::GridSpan& span,
		    int cellX, int cellY) const;

private:
	int ring_;
};

}
