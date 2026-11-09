#pragma once

#include "world/BuildPlaceCatalog.h"
#include "world/ScenePixelMapper.h"

namespace world {

// Geometría pura del marcador de selección de lugar: el rectángulo
// donde se dibuja el bloque sobre la celda seleccionada (la escala del
// estado, centrada en la celda). Una sola fuente para el painter y el
// culling: lo que se toca coincide con lo que se ve (§9). Sin SDL.
class BuildPlaceLayout {
public:
	world::PixelRect rectFor(const world::PixelRect& cellRect,
				const world::BuildPlaceVisual& visual) const;
};

}
