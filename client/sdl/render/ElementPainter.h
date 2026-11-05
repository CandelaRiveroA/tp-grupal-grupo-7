#pragma once

#include <map>

#include "world/ElementLayout.h"
#include "world/SceneDescription.h"
#include "world/ScenePixelMapper.h"

namespace sdl {

class SpriteSheet;

class ElementPainter {
public:
	ElementPainter(const world::TileSetDef& tileset,
		       const world::ElementPlacement& element,
		       const world::PixelRect& area);

	world::PixelRect bounds() const;

	// Dibuja la pieza del elemento dentro del clip. `firstRow` /
	// `lastRow` acotan qué filas de la pieza se dibujan (0-based,
	// inclusive): la pieza entera es el rango completo; la capa frontal
	// de un elemento "entrable" son solo sus filas de adelante.
	void paint(SpriteSheet& sheet, const world::PixelRect& clip,
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

	const world::ElementLayout layout_;
	std::map<world::DrawMode, PaintMethod> paintMethods_;
};

}  // namespace sdl
