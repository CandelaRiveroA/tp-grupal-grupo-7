#pragma once

#include "sdl/render/TileLayout.h"
#include "world/SceneDescription.h"
#include "world/ScenePixelMapper.h"

namespace sdl {

class SpriteSheet;

class ElementPainter {
public:
	ElementPainter(const world::TileSetDef& tileset,
		       const world::ElementPlacement& element,
		       const world::PixelRect& area);

	void paint(SpriteSheet& sheet) const;

private:
	void paintSingle(SpriteSheet& sheet) const;
	void paintTiled(SpriteSheet& sheet) const;
	void paintStrip(SpriteSheet& sheet) const;

	void blit(SpriteSheet& sheet, const TileLayout::Placement& piece) const;

	int pieceWidth() const;
	int pieceHeight() const;

	const world::TileSetDef& tileset_;
	const world::ElementPlacement& element_;
	const world::PixelRect area_;
};

}
