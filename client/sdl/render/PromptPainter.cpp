#include "sdl/render/PromptPainter.h"

#include <cstddef>
#include <string>
#include <utility>

#include "core/Constants.h"
#include "sdl/render/TextRenderer.h"
#include "world/WorldConstants.h"

namespace sdl {

PromptPainter::PromptPainter(TextRenderer& text,
			     const world::PromptLayout& layout,
			     ButtonPainter& buttons)
	: drawMethods_{
		  {game::PopupView::Question, &PromptPainter::drawQuestion},
		  {game::PopupView::Waiting, &PromptPainter::drawWaiting},
		  {game::PopupView::NextDay, &PromptPainter::drawNextDay},
		  // Los regalos reutilizan las mismas formas de dibujo: el
		  // copy y el botón salen de su PromptCopy.
		  {game::PopupView::GiftOffer, &PromptPainter::drawQuestion},
		  {game::PopupView::GiftWaiting, &PromptPainter::drawWaiting},
		  {game::PopupView::GiftReceived, &PromptPainter::drawQuestion},
		  {game::PopupView::Hidden, &PromptPainter::drawHidden},
	  }
	, text_(text)
	, layout_(layout)
	, buttons_(buttons) {}

void PromptPainter::draw(const game::PopupState& state,
			 const int hoveredOption,
			 const world::PixelBox& panel) {
	const DrawMethod method = drawMethods_.at(state.view());
	(this->*method)(state, hoveredOption, panel);
}

void PromptPainter::drawQuestion(const game::PopupState& state,
				 const int hoveredOption,
				 const world::PixelBox& panel) {
	const world::PromptCopy& copy = layout_.prompt(state.promptId());
	const std::vector<std::pair<std::string, std::string>> tokens = {
		{"{0}", state.giftSenderName()},
	};
	text_.draw(fillTokens(copy.text, tokens),
		   layout_.questionBox(panel), layout_.textColor);
	const std::vector<world::PixelBox> boxes = layout_.optionBoxes(
		panel, copy);
	for (std::size_t i = 0; i < boxes.size() && i < copy.options.size();
	     ++i) {
		const bool hovered = static_cast<int>(i) == hoveredOption;
		text_.draw(copy.options[i], boxes[i],
			   hovered ? layout_.hoverColor : layout_.textColor);
	}
}

void PromptPainter::drawWaiting(const game::PopupState& state, const int,
				const world::PixelBox& panel) {
	const world::PromptCopy& copy = layout_.prompt(state.promptId());
	const std::vector<std::pair<std::string, std::string>> tokens = {
		{"{0}", std::to_string(state.waitingReady())},
		{"{1}", std::to_string(state.waitingTotal())},
	};
	text_.draw(fillTokens(copy.text, tokens), layout_.waitingBox(panel),
		   layout_.textColor);
	// Un muerto no cancela: en el sueño el botón no aparece.
	if (!copy.button.empty() &&
	    (state.view() != game::PopupView::Waiting || state.waitingCancelable())) {
		drawButton(copy.button, panel);
	}
}

void PromptPainter::drawNextDay(const game::PopupState& state, const int,
				const world::PixelBox& panel) {
	const world::PromptCopy& copy = layout_.prompt(state.promptId());
	text_.draw(copy.text, layout_.nextDayBox(panel), layout_.textColor);
	if (!copy.button.empty()) {
		drawButton(copy.button, panel);
	}
}

void PromptPainter::drawHidden(const game::PopupState&, const int,
			       const world::PixelBox&) {}

void PromptPainter::drawButton(const std::string& name,
			       const world::PixelBox& panel) {
	buttons_.draw(name, layout_.buttonBox(name, panel));
}

std::string PromptPainter::fillTokens(
	const std::string& text,
	const std::vector<std::pair<std::string, std::string>>& tokens) const {
	std::string result = text;
	for (const std::pair<std::string, std::string>& token : tokens) {
		const std::size_t at = result.find(token.first);
		if (at != std::string::npos) {
			result.replace(at, token.first.size(), token.second);
		}
	}
	return result;
}

}  // namespace sdl
