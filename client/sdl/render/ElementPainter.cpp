#include "sdl/render/ElementPainter.h"

#include <algorithm>

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
			   const world::PixelRect& clip, const int firstRow,
			   const int lastRow) const {
	const PaintMethod method = paintMethods_.at(layout_.mode());
	(this->*method)(sheet, clip, firstRow, lastRow);
}

void ElementPainter::paintSingle(SpriteSheet& sheet,
				 const world::PixelRect&, const int firstRow,
				 const int lastRow) const {
	const world::PixelRect box = layout_.bounds();
	if (box.width <= 0 || box.height <= 0) {
		return;
	}
	blit(sheet, 0, 0, box.width, box.height, firstRow, lastRow);
}

void ElementPainter::paintTiled(SpriteSheet& sheet,
				const world::PixelRect& clip,
				const int firstRow, const int lastRow) const {
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
			     piece.height, firstRow, lastRow);
		}
	}
}

void ElementPainter::paintStrip(SpriteSheet& sheet,
				const world::PixelRect& clip, const int firstRow,
				const int lastRow) const {
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
		     piece.height, firstRow, lastRow);
	}
}

void ElementPainter::blit(SpriteSheet& sheet, const int offsetX,
			  const int offsetY, const int width,
			  const int height, const int firstRow,
			  const int lastRow) const {
	if (width <= 0 || height <= 0) {
		return;
	}
	// La pieza se dibuja celda por celda en su cuadrante del destino:
	// sirve igual para el bloque contiguo (region) y para las
	// composiciones explícitas (cells). La pieza cortada por el clip se
	// dibuja con la fracción de fuente que se ve (la misma regla que
	// tenía drawBlock con fracW/fracH). Solo se dibujan las filas del
	// rango pedido, cada una en su posición natural.
	const world::ElementPlacement& element = layout_.element();
	const int cellW = layout_.pieceWidth() / element.pieceSize.columns;
	const int cellH = layout_.pieceHeight() / element.pieceSize.rows;
	for (int row = 0; row < element.pieceSize.rows; ++row) {
		if (row < firstRow || row > lastRow) {
			continue;
		}
		for (int column = 0; column < element.pieceSize.columns;
		     ++column) {
			const int visibleW =
				std::min(cellW, width - column * cellW);
			const int visibleH =
				std::min(cellH, height - row * cellH);
			if (visibleW <= 0 || visibleH <= 0) {
				continue;
			}
			const world::GridCell cell =
				element.pieceCells[static_cast<std::size_t>(
					row * element.pieceSize.columns +
					column)];
			sheet.drawBlock(
				cell.column, cell.row, 1, 1,
				SDL2pp::Rect(layout_.area().x + offsetX +
						     column * cellW,
					     layout_.area().y + offsetY +
						     row * cellH,
					     visibleW, visibleH),
				Flip::None,
				static_cast<double>(visibleW) / cellW,
				static_cast<double>(visibleH) / cellH);
		}
	}
}

}  // namespace sdl
