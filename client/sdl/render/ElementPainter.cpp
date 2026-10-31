#include "sdl/render/ElementPainter.h"

#include "sdl/render/SpriteSheet.h"
#include "sdl/render/TileLayout.h"

namespace sdl {

ElementPainter::ElementPainter(const world::TileSetDef& tileset,
			       const world::ElementPlacement& element,
			       const world::PixelRect& area)
	: layout_(tileset, element, area)
	, paintMethods_{
		  {world::DrawMode::Single, &ElementPainter::paintSingle},
		  {world::DrawMode::Tile, &ElementPainter::paintTiled},
		  {world::DrawMode::HorizontalStrip,
		   &ElementPainter::paintStrip},
	  } {}

world::PixelRect ElementPainter::bounds() const { return layout_.bounds(); }

void ElementPainter::paint(SpriteSheet& sheet,
			   const world::PixelRect& clip) const {
	const PaintMethod method = paintMethods_.at(layout_.mode());
	(this->*method)(sheet, clip);
}

void ElementPainter::paintSingle(SpriteSheet& sheet,
				 const world::PixelRect&) const {
	const world::PixelRect box = layout_.bounds();
	if (box.width <= 0 || box.height <= 0) {
		return;
	}
	const world::ElementPlacement& element = layout_.element();
	sheet.drawBlock(element.region.origin.column,
			element.region.origin.row,
			element.region.size.columns,
			element.region.size.rows,
			SDL2pp::Rect(box.x, box.y, box.width, box.height));
}

void ElementPainter::paintTiled(SpriteSheet& sheet,
				const world::PixelRect& clip) const {
	TileLayout layout(layout_.area(), layout_.pieceWidth(),
			  layout_.pieceHeight());
	const TileLayout::Range range = layout.visibleTiled(clip);
	if (range.empty()) {
		return;
	}
	for (int row = range.firstRow; row <= range.lastRow; ++row) {
		for (int column = range.firstColumn; column <= range.lastColumn;
		     ++column) {
			const TileLayout::Placement piece =
				layout.tiledAt(column, row);
			blit(sheet, piece.offsetX, piece.offsetY, piece.width,
			     piece.height);
		}
	}
}

void ElementPainter::paintStrip(SpriteSheet& sheet,
				const world::PixelRect& clip) const {
	TileLayout layout(layout_.area(), layout_.pieceWidth(),
			  layout_.pieceHeight());
	const TileLayout::Range range = layout.visibleStrip(clip);
	if (range.empty()) {
		return;
	}
	for (int column = range.firstColumn; column <= range.lastColumn;
	     ++column) {
		const TileLayout::Placement piece = layout.stripAt(column);
		blit(sheet, piece.offsetX, piece.offsetY, piece.width,
		     piece.height);
	}
}

void ElementPainter::blit(SpriteSheet& sheet, const int offsetX,
			  const int offsetY, const int width,
			  const int height) const {
	if (width <= 0 || height <= 0) {
		return;
	}
	const world::ElementPlacement& element = layout_.element();
	sheet.drawBlock(element.region.origin.column,
			element.region.origin.row,
			element.region.size.columns,
			element.region.size.rows,
			SDL2pp::Rect(layout_.area().x + offsetX,
				     layout_.area().y + offsetY, width, height),
			Flip::None,
			static_cast<double>(width) / layout_.pieceWidth(),
			static_cast<double>(height) / layout_.pieceHeight());
}

}  // namespace sdl
