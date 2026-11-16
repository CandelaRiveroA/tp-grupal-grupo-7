#include "sdl/render/PromptPainter.h"

#include <cstddef>
#include <string>
#include <tuple>
#include <utility>

#include "core/Constants.h"
#include "sdl/render/TextRenderer.h"
#include "world/WorldConstants.h"

namespace sdl {

PromptPainter::PromptPainter(SdlContext& context, TextRenderer& text,
			     const world::PromptLayout& layout)
	: drawMethods_{
		  {game::PopupView::Question, &PromptPainter::drawQuestion},
		  {game::PopupView::Waiting, &PromptPainter::drawWaiting},
		  {game::PopupView::NextDay, &PromptPainter::drawNextDay},
		  {game::PopupView::Hidden, &PromptPainter::drawHidden},
	  }
	, text_(text)
	, layout_(layout) {
	// Una hoja por botón del layout (hoy ambos usan cursors.png).
	for (const auto& [name, frame] : layout.buttons) {
		buttonSheets_.emplace(
			std::piecewise_construct, std::forward_as_tuple(name),
			std::forward_as_tuple(context,
					      core::assetPath(frame.sheet),
					      frame.cellWidth,
					      frame.cellHeight));
	}
}

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
	// Un muerto no cancela: el botón no aparece.
	if (state.waitingCancelable()) {
		drawButton(world::kPromptCancelButton, panel);
	}
}

void PromptPainter::drawNextDay(const game::PopupState&, const int,
				const world::PixelBox& panel) {
	text_.draw(layout_.nextDay, layout_.nextDayBox(panel),
		   layout_.textColor);
	drawButton(world::kPromptOkButton, panel);
}

void PromptPainter::drawHidden(const game::PopupState&, const int,
			       const world::PixelBox&) {}

void PromptPainter::drawButton(const std::string& name,
			       const world::PixelBox& panel) {
	// Un botón del popup: las 9 piezas de su 9-slice estiradas a su
	// caja (esquinas, bordes y relleno).
	SpriteSheet& sheet = buttonSheets_.at(name);
	const world::PixelBox button = layout_.buttonBox(name, panel);
	for (const world::FrameLayout::Piece& piece :
	     layout_.buttonPieces(name, button)) {
		sheet.drawBlock(
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
