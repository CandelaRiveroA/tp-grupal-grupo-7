#pragma once

#include <map>

#include "world/ElementLayout.h"
#include "world/SceneDescription.h"
#include "world/ScenePixelMapper.h"

namespace sdl {

class SpriteSheet;
class Viewport;

class ElementPainter {
public:
	// `area` y toda la geometría de ElementLayout/TileLayout están en
	// píxeles de mundo. El painter recién convierte a pantalla al dibujar.
	ElementPainter(const world::TileSetDef& tileset,
		       const world::ElementPlacement& element,
		       const world::PixelRect& area,
		       const Viewport& viewport);

	// Bounds reales del elemento en píxeles de pantalla (para el culling
	// por contrato de §11).
	world::PixelRect screenBounds() const;

	// Dibuja la pieza del elemento dentro del clip en píxeles de mundo.
	// `firstRow` / `lastRow` acotan qué filas de la pieza se dibujan
	// (0-based, inclusive): la pieza entera es el rango completo; la capa
	// frontal de un elemento "entrable" son solo sus filas de adelante.
	void paint(SpriteSheet& sheet, const world::PixelRect& worldClip,
		   int firstRow, int lastRow) const;

private:
	using PaintMethod =
		void (ElementPainter::*)(SpriteSheet&,
					 const world::PixelRect&, int,
					 int) const;

	void paintSingle(SpriteSheet& sheet, const world::PixelRect& clip,
			 int firstRow, int lastRow) const;
	void paintTiled(SpriteSheet& sheet, const world::PixelRect& clip,
			int firstRow, int lastRow) const;
	void paintStrip(SpriteSheet& sheet, const world::PixelRect& clip,
			int firstRow, int lastRow) const;

	void blit(SpriteSheet& sheet, int offsetX, int offsetY, int width,
		  int height, int firstRow, int lastRow) const;

	const Viewport& viewport_;
	const world::ElementLayout layout_;
	std::map<world::DrawMode, PaintMethod> paintMethods_;
};

}  // namespace sdl
