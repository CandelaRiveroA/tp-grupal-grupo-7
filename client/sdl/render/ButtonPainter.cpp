#include "sdl/render/ButtonPainter.h"

#include <tuple>
#include <utility>

#include "core/Constants.h"
#include "sdl/context/SdlContext.h"

namespace sdl {

ButtonPainter::ButtonPainter(SdlContext& context,
			     const world::PromptLayout& layout)
	: layout_(layout) {
	// Una hoja por botón del layout (hoy todos usan cursors.png).
	for (const auto& [name, frame] : layout.buttons) {
		sheets_.emplace(
			std::piecewise_construct, std::forward_as_tuple(name),
			std::forward_as_tuple(context,
					      core::assetPath(frame.sheet),
					      frame.cellWidth,
					      frame.cellHeight));
	}
}

void ButtonPainter::draw(const std::string& name,
			 const world::PixelBox& box) {
	// Las 9 piezas del 9-slice estiradas a la caja (esquinas, bordes y
	// relleno).
	SpriteSheet& sheet = sheets_.at(name);
	for (const world::FrameLayout::Piece& piece :
	     layout_.buttonPieces(name, box)) {
		sheet.drawBlock(
			piece.cell.col, piece.cell.row, piece.srcCols,
			piece.srcRows,
			SDL2pp::Rect(piece.dest.x, piece.dest.y,
				     piece.dest.width, piece.dest.height));
	}
}

}  // namespace sdl
