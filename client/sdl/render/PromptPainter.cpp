#include "sdl/render/PromptPainter.h"

#include <cstddef>
#include <string>
#include <utility>

#include "core/Constants.h"
#include "sdl/render/TextRenderer.h"

namespace sdl {

PromptPainter::PromptPainter(SdlContext& context, TextRenderer& text,
			     const world::PromptLayout& layout)
	: drawMethods_{
		  {game::PopupView::Question, &PromptPainter::drawQuestion},
		  {game::PopupView::Waiting, &PromptPainter::drawWaiting},
		  {game::PopupView::Hidden, &PromptPainter::drawHidden},
	  }
	, text_(text)
	, layout_(layout)
	, cancelSheet_(context, core::assetPath(layout.cancel.sheet),
		       layout.cancel.cellWidth, layout.cancel.cellHeight) {}

void PromptPainter::draw(const game::PopupState& state,
			 const int hoveredOption,
			 const world::PixelBox& panel) {
	const DrawMethod method = drawMethods_.at(state.view());
	(this->*method)(state, hoveredOption, panel);
}

void PromptPainter::drawQuestion(const game::PopupState&,
				 const int hoveredOption,
				 const world::PixelBox& panel) {
	text_.draw(layout_.question, layout_.questionBox(panel),
		   layout_.textColor);
	const std::vector<world::PixelBox> boxes = layout_.optionBoxes(panel);
	for (std::size_t i = 0; i < boxes.size() && i < layout_.options.size();
	     ++i) {
		const bool hovered = static_cast<int>(i) == hoveredOption;
		text_.draw(layout_.options[i], boxes[i],
			   hovered ? layout_.hoverColor : layout_.textColor);
	}
}

void PromptPainter::drawWaiting(const game::PopupState& state, const int,
				const world::PixelBox& panel) {
	text_.draw(waitingText(state), layout_.waitingBox(panel),
		   layout_.textColor);
	drawCancel(panel);
}

void PromptPainter::drawHidden(const game::PopupState&, const int,
			       const world::PixelBox&) {}

void PromptPainter::drawCancel(const world::PixelBox& panel) {
	// El botón de cancelar: las 9 piezas de su 9-slice estiradas a su
	// caja (esquinas, bordes y relleno).
	const world::PixelBox button = layout_.cancelBox(panel);
	for (const world::FrameLayout::Piece& piece :
	     layout_.cancelPieces(button)) {
		cancelSheet_.drawBlock(
			piece.cell.col, piece.cell.row, piece.srcCols,
			piece.srcRows,
			SDL2pp::Rect(piece.dest.x, piece.dest.y,
				     piece.dest.width, piece.dest.height));
	}
}

std::string PromptPainter::waitingText(const game::PopupState& state) const {
	std::string text = layout_.waiting;
	const std::pair<const char*, std::string> tokens[] = {
		{"{0}", std::to_string(state.waitingReady())},
		{"{1}", std::to_string(state.waitingTotal())},
	};
	for (const std::pair<const char*, std::string>& token : tokens) {
		const std::size_t at = text.find(token.first);
		if (at != std::string::npos) {
			text.replace(at, std::string(token.first).size(),
				     token.second);
		}
	}
	return text;
}

}  // namespace sdl
