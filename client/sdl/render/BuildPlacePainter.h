#pragma once

#include "sdl/render/SheetCache.h"
#include "world/BuildPlaceCatalog.h"
#include "world/Grid.h"
#include "world/ScenePixelMapper.h"

namespace sdl {
class SdlContext;
class Viewport;
}

namespace sdl {

// Pinta el cuadrado de selección de lugar sobre una celda: el bloque de
// su estado, estirado sobre la celda (world::BuildPlaceLayout). Solo
// dibuja: QUÉ celda y en qué estado la marca lo decide
// game::PlantingSelection; CÓMO se ve, el catálogo. Los rectángulos
// pasan por el viewport como todo dibujo de mundo (§9/§11).
class BuildPlacePainter {
public:
	explicit BuildPlacePainter(SdlContext& context);

	// Dibuja el marcador del estado sobre la celda. No hace nada si el
	// estado no tiene bloque cargado.
	void draw(const world::BuildPlaceCatalog& catalog,
		  world::BuildPlaceState state, const world::GridCell& cell,
		  const world::ScenePixelMapper& mapper,
		  const Viewport& viewport);

private:
	SheetCache sheets_;
};

}
