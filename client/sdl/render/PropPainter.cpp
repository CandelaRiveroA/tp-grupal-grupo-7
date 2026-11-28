#include "sdl/render/PropPainter.h"

#include <SDL2pp/SDL2pp.hh>

#include "core/Constants.h"
#include "sdl/render/SheetCache.h"
#include "sdl/render/SpriteSheet.h"
#include "sdl/render/Viewport.h"

namespace sdl {

PropPainter::PropPainter(SheetCache& sheets) : sheets_(sheets) {}

void PropPainter::draw(const world::NpcDef& def, const int cellPixelSize,
		       const PropDraw& how, const Viewport& viewport) {
	const world::PixelRect whole = def.body.box(cellPixelSize, how.where);
	world::PixelRect body = whole;
	body.x += how.shift.x;
	body.y += how.shift.y;
	world::PixelRect screenBody = viewport.toScreen(body);
	if (!def.body.zoom) {
		screenBody = world::PixelRect{
			screenBody.x + (screenBody.width - body.width) / 2,
			screenBody.y + (screenBody.height - body.height) / 2,
			body.width, body.height};
	}
	if (!viewport.intersects(screenBody) || def.body.span.rows <= 0) {
		return;
	}
	SpriteSheet& sheet = sheets_.sheet(
		def.body.sheet, core::assetPath(def.body.sheet),
		def.body.cellWidth, def.body.cellHeight);
	const int lastRow = how.lastRow < 0 ? def.body.span.rows - 1 : how.lastRow;
	const int pieceHeight = whole.height / def.body.span.rows;
	sheet.setAlphaMod(static_cast<std::uint8_t>(how.alpha));
	for (const world::NpcBodyPiece& piece :
	     def.body.pieces(cellPixelSize, how.where, how.frame)) {
		const int row = pieceHeight > 0 ? (piece.dest.y - whole.y) / pieceHeight : 0;
		if (row < how.firstRow || row > lastRow) {
			continue;
		}
		world::PixelRect dest;
		if (def.body.zoom) {
			dest = piece.dest;
			dest.x += how.shift.x;
			dest.y += how.shift.y;
			dest = viewport.toScreen(dest);
		} else {
			dest = world::PixelRect{
				screenBody.x + (piece.dest.x - whole.x),
				screenBody.y + (piece.dest.y - whole.y),
				piece.dest.width, piece.dest.height};
		}
		sheet.drawBlock(piece.cell.column, piece.cell.row, 1, 1,
				SDL2pp::Rect(dest.x, dest.y, dest.width, dest.height));
	}
	sheet.clearAlphaMod();
}

}
