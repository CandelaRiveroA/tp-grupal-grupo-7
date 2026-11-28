#pragma once

#include <string>

#include "sdl/render/PropPainter.h"
#include "world/FurnitureGeometry.h"
#include "world/Grid.h"
#include "world/ScenePixelMapper.h"

namespace sdl {

class SheetCache;
class Viewport;

// Pinta la vista previa de un mueble por colocar: su sprite (el mismo
// del mundo, bloque o por piezas) semitransparente sobre la celda base y
// adelantado hacia el lado elegido. Solo dibuja: DÓNDE y si es válido lo
// decide game::PlacementSelection; la forma sale de FurnitureGeometry.
class FurnitureGhostPainter {
public:
	FurnitureGhostPainter(SheetCache& sheets,
			      const world::FurnitureGeometry& furniture);

	// `toward`/`push`: el sprite se adelanta `push` celdas hacia ese lado.
	void draw(const std::string& contentId, const world::GridCell& base,
		  world::FurnitureFacing facing, const world::GridCell& toward, double push, int alpha,
		  const world::ScenePixelMapper& mapper,
		  const Viewport& viewport);

private:
	SheetCache& sheets_;
	const world::FurnitureGeometry& furniture_;
	PropPainter props_;
};

}
