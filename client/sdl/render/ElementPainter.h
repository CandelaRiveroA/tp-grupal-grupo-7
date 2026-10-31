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

	void paint(SpriteSheet& sheet, const world::PixelRect& clip) const;

private:
	using PaintMethod =
		void (ElementPainter::*)(SpriteSheet&,
					 const world::PixelRect&) const;

	void paintSingle(SpriteSheet& sheet,
			 const world::PixelRect& clip) const;
	void paintTiled(SpriteSheet& sheet,
			const world::PixelRect& clip) const;
	void paintStrip(SpriteSheet& sheet,
			const world::PixelRect& clip) const;

	void blit(SpriteSheet& sheet, int offsetX, int offsetY, int width,
		  int height) const;

	const world::ElementLayout layout_;
	std::map<world::DrawMode, PaintMethod> paintMethods_;
};

}  // namespace sdl
