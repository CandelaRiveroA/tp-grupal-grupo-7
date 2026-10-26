#include "sdl/render/ElementPainter.h"

#include "sdl/render/SpriteSheet.h"

namespace sdl {

ElementPainter::ElementPainter(const world::TileSetDef& tileset,
			       const world::ElementPlacement& element,
			       const world::PixelRect& area)
	: tileset_(tileset)
	, element_(element)
	, area_(area) {}

void ElementPainter::paint(SpriteSheet& sheet) const {
	switch (element_.draw.mode) {
	case world::DrawMode::Single:
		paintSingle(sheet);
		return;
	case world::DrawMode::Tile:
		paintTiled(sheet);
		return;
	case world::DrawMode::HorizontalStrip:
		paintStrip(sheet);
		return;
	}
}

void ElementPainter::paintSingle(SpriteSheet& sheet) const {
	const SDL2pp::Rect dest(area_.x, area_.y, pieceWidth(),
				pieceHeight());
	sheet.drawBlock(element_.region.origin.column,
			element_.region.origin.row,
			element_.region.size.columns,
			element_.region.size.rows, dest);
}

void ElementPainter::paintTiled(SpriteSheet& sheet) const {
	const TileLayout layout(area_, pieceWidth(), pieceHeight());
	for (const TileLayout::Placement& piece : layout.tiled()) {
		blit(sheet, piece);
	}
}

void ElementPainter::paintStrip(SpriteSheet& sheet) const {
	const TileLayout layout(area_, pieceWidth(), pieceHeight());
	for (const TileLayout::Placement& piece : layout.strip()) {
		blit(sheet, piece);
	}
}

void ElementPainter::blit(SpriteSheet& sheet,
			  const TileLayout::Placement& piece) const {
	const SDL2pp::Rect dest(area_.x + piece.offsetX,
				area_.y + piece.offsetY, piece.width,
				piece.height);
	sheet.drawBlock(element_.region.origin.column,
			element_.region.origin.row,
			element_.region.size.columns,
			element_.region.size.rows, dest, Flip::None,
			static_cast<double>(piece.width) / pieceWidth(),
			static_cast<double>(piece.height) / pieceHeight());
}

int ElementPainter::pieceWidth() const {
	return tileset_.cellWidth * element_.region.size.columns *
	       element_.draw.scaleX;
}

int ElementPainter::pieceHeight() const {
	return tileset_.cellHeight * element_.region.size.rows *
	       element_.draw.scaleY;
}

}
