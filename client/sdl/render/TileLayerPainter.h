#pragma once

#include "world/SceneDescription.h"
#include "world/ScenePixelMapper.h"

namespace sdl {

class SheetCache;
class Viewport;

// Dibuja las capas de piso de una escena (una celda de arte por celda
// lógica). QUÉ celdas se ven lo resuelve world::TileLayerLayout; esto
// solo pinta cada una con la hoja cacheada, ya en pantalla.
class TileLayerPainter {
public:
	explicit TileLayerPainter(SheetCache& sheets);

	void draw(const world::SceneDescription& scene,
		  const world::ScenePixelMapper& mapper,
		  const Viewport& viewport);

private:
	void drawLayer(const world::TileLayer& layer,
		       const world::SceneDescription& scene,
		       const world::ScenePixelMapper& mapper,
		       const Viewport& viewport);

	SheetCache& sheets_;
};

}  // namespace sdl
