#pragma once

#include "world/Grid.h"
#include "world/ScenePixelMapper.h"
#include "world/SpriteVisual.h"

namespace world {

// Geometría pura de un mueble colocado: la caja (px de mundo) que ocupa
// su sprite en la grilla. La celda que manda la fuente de estado es la
// BASE del mueble (abajo-izquierda): el sprite crece hacia la derecha y
// hacia arriba, tantas celdas como abarque su visual (`span`). Usa el
// ScenePixelMapper (única conversión celda <-> px del proyecto).
class FurnitureLayout {
public:
	explicit FurnitureLayout(const ScenePixelMapper& mapper);

	PixelRect box(const SpriteVisual& visual, const GridCell& base) const;

private:
	ScenePixelMapper mapper_;
};

}
