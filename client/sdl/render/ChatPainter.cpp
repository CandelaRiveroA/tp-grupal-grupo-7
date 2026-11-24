#include "sdl/render/ChatPainter.h"

#include <algorithm>

#include "core/Constants.h"
#include "sdl/context/SdlContext.h"
#include "sdl/render/TextRenderer.h"
#include "world/ChatPanel.h"

namespace sdl {

ChatPainter::ChatPainter(SdlContext& context, TextRenderer& text,
			 const world::ChatLayout& layout)
	: context_(context)
	, text_(text)
	, layout_(layout)
	, sheet_(context, core::assetPath(layout.sheetImage),
		 layout.sheetCellWidth, layout.sheetCellHeight) {}

int ChatPainter::textInset() const {
	// Incluye el marco: el texto no se escribe encima del borde.
	return layout_.sheetBorder + layout_.textPad;
}

std::string ChatPainter::withSender(const game::ViewChatLine& line) {
	const std::string sender = line.characterName.empty()
					   ? "P" + std::to_string(line.playerId)
					   : line.characterName;
	return sender + ": " + line.text;
}

int ChatPainter::contentWidth() const {
	return std::max(1, world::chatBoxWidth(context_.width(), layout_) -
				  2 * textInset());
}

const std::vector<std::unique_ptr<SDL2pp::Texture>>& ChatPainter::
	historyTextures(const std::vector<game::ViewChatLine>& log) {
	std::vector<std::string> key;
	key.reserve(log.size());
	for (const game::ViewChatLine& line: log) {
		key.push_back(withSender(line));
	}
	if (key != historyKey_) {
		historyKey_ = std::move(key);
		history_.clear();
		history_.reserve(historyKey_.size());
		for (const std::string& text: historyKey_) {
			history_.push_back(std::make_unique<SDL2pp::Texture>(
				text_.renderWrapped(text, layout_.fontSize,
						    layout_.textColor,
						    contentWidth(), layout_.fontFace)));
		}
	}
	return history_;
}

SDL2pp::Texture& ChatPainter::inputTexture(const std::string& inputText,
					   const bool cursorOn) {
	// El cursor es un carácter más: el blink es renderizar con/sin él.
	// Con texto vacío, un espacio mantiene la caja de una línea.
	std::string shown = inputText.empty() ? " " : inputText;
	if (cursorOn) {
		shown += "|";
	}
	if (shown != inputKey_) {
		inputKey_ = shown;
		input_ = std::make_unique<SDL2pp::Texture>(
			text_.renderWrapped(shown, layout_.fontSize,
					    layout_.textColor, contentWidth(),
					    layout_.fontFace));
	}
	return *input_;
}

void ChatPainter::drawBox(const world::PixelBox& box) {
	if (box.width <= 0 || box.height <= 0) {
		return;
	}
	sheet_.drawNineSlice(0, 0, layout_.sheetBorder,
			     SDL2pp::Rect(box.x, box.y, box.width, box.height));
}

void ChatPainter::drawHistory(const world::PixelBox& box,
			      const std::uint8_t textAlpha) {
	if (history_.empty()) {
		return;
	}
	const int x = box.x + textInset();
	const int ceiling = box.y + layout_.textPad;
	int bottom = box.y + box.height - layout_.textPad;
	// De abajo hacia arriba: el más nuevo pegado a la caja de tipeo.
	for (std::size_t i = history_.size(); i-- > 0;) {
		SDL2pp::Texture& tex = *history_[i];
		tex.SetAlphaMod(textAlpha);
		const SDL2pp::Point size = tex.GetSize();
		if (bottom - size.y < ceiling) {
			if (i + 1 == history_.size() && bottom > ceiling) {
				// El recién llegado no desaparece: sus
				// primeros renglones, recortados al tope
				// (así hace el preview).
				const int fit =
					std::min(size.y, bottom - ceiling);
				context_.drawTexture(
					tex, SDL2pp::Rect(0, 0, size.x, fit),
					SDL2pp::Rect(x, ceiling, size.x, fit));
			}
			break;
		}
		context_.drawTexture(tex, SDL2pp::Rect(0, 0, size.x, size.y),
				     SDL2pp::Rect(x, bottom - size.y, size.x,
						  size.y));
		bottom -= size.y;
	}
}

void ChatPainter::drawOpen(const std::string& inputText,
			   const std::vector<game::ViewChatLine>& log,
			   const bool cursorOn) {
	SDL2pp::Texture& input = inputTexture(inputText, cursorOn);
	const std::vector<std::unique_ptr<SDL2pp::Texture>>& history =
		historyTextures(log);

	std::vector<int> heights;
	heights.reserve(history.size());
	for (const auto& tex: history) {
		heights.push_back(tex->GetSize().y);
	}
	const world::ChatPanel panels = world::chatPanels(
		context_.width(), context_.height(), input.GetSize().y,
		heights, layout_);

	if (panels.history.height > 0) {
		drawBox(panels.history);
		drawHistory(panels.history, 255);
	}

	if (panels.input.height > 0) {
		drawBox(panels.input);
		// Si el tipeo excede la caja se ven los ÚLTIMOS renglones
		const SDL2pp::Point size = input.GetSize();
		const int shown = std::min(
			size.y, panels.input.height - 2 * layout_.textPad);
		context_.drawTexture(
			input,
			SDL2pp::Rect(0, size.y - shown, size.x, shown),
			SDL2pp::Rect(panels.input.x + textInset(),
				     panels.input.y + layout_.textPad,
				     size.x, shown));
	}
}

void ChatPainter::drawPreview(const std::vector<game::ViewChatLine>& log) {
	if (log.empty()) {
		return;
	}
	historyTextures(log);
	const SDL2pp::Point size = history_.back()->GetSize();
	const int budget = layout_.previewRows * layout_.lineHeight();
	const world::ChatPanel panels = world::chatPanels(
		context_.width(), context_.height(), 0,
		{std::min(size.y, budget)}, layout_);
	if (panels.history.height > 0) {
		drawBox(panels.history);
		drawHistory(panels.history,
			    static_cast<std::uint8_t>(layout_.previewAlpha));
	}
}

}
