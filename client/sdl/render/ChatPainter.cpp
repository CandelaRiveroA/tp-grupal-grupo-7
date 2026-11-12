#include "sdl/render/ChatPainter.h"

#include <algorithm>
#include <cstddef>

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

world::TextMeasure ChatPainter::measure() const {
	return [this](const std::string& text) {
		return text_.textWidth(text, layout_.fontSize);
	};
}

std::vector<std::vector<std::string>> ChatPainter::wrapMessages(
	const std::vector<game::ViewChatLine>& log) const {
	const world::TextMeasure measureText = measure();
	const int width = contentWidth();
	std::vector<std::vector<std::string>> messages;
	messages.reserve(log.size());
	for (const game::ViewChatLine& line: log) {
		messages.push_back(world::wrapChatText(withSender(line), width, measureText));
	}
	return messages;
}

void ChatPainter::drawBox(const world::PixelBox& box) {
	if (box.width <= 0 || box.height <= 0) {
		return;
	}
	sheet_.drawNineSlice(0, 0, layout_.sheetBorder,
			     SDL2pp::Rect(box.x, box.y, box.width, box.height));
}

void ChatPainter::drawLine(const world::PixelBox& panel, const int row,
			   const std::string& text, const std::uint8_t textAlpha) {
	world::PixelBox line;
	line.x = panel.x + textInset();
	line.y = panel.y + layout_.textPad + row * layout_.lineHeight();
	line.width = panel.width - 2 * textInset();
	line.height = layout_.lineHeight();
	world::Color color = layout_.textColor;
	color.a = textAlpha;
	text_.drawLeft(text, line, color, layout_.fontSize);
}

void ChatPainter::drawLines(const std::vector<std::string>& inputLines,
			    const std::vector<std::string>& historyLines,
			    const bool cursorOn, const std::uint8_t textAlpha) {
	const world::ChatPanel panels = world::chatPanels(
		context_.width(), context_.height(),
		static_cast<int>(inputLines.size()),
		static_cast<int>(historyLines.size()), layout_);

	if (panels.history.height > 0) {
		drawBox(panels.history);
		for (std::size_t i = 0; i < historyLines.size(); ++i) {
			drawLine(panels.history, static_cast<int>(i), historyLines[i],
				 textAlpha);
		}
	}

	if (panels.input.height > 0) {
		// Si el tipeo excede la caja se ven los ÚLTIMOS renglones (los
		// del cursor).
		const std::size_t rows =
			static_cast<std::size_t>(panels.inputVisibleRows);
		const std::size_t first =
			inputLines.size() > rows ? inputLines.size() - rows : 0;
		drawBox(panels.input);
		for (std::size_t i = first; i < inputLines.size(); ++i) {
			drawLine(panels.input, static_cast<int>(i - first),
				 inputLines[i], textAlpha);
		}

		if (cursorOn && !inputLines.empty()) {
			const int row =
				static_cast<int>(inputLines.size() - 1 - first);
			const int x = panels.input.x + textInset() +
				      text_.textWidth(inputLines.back(),
						      layout_.fontSize);
			const int y = panels.input.y + layout_.textPad +
				      row * layout_.lineHeight() +
				      (layout_.lineHeight() - layout_.fontSize) / 2;
			context_.fillRect(
				SDL2pp::Rect(x, y, 2, layout_.fontSize),
				SDL2pp::Color(
					static_cast<Uint8>(layout_.textColor.r),
					static_cast<Uint8>(layout_.textColor.g),
					static_cast<Uint8>(layout_.textColor.b),
					textAlpha));
		}
	}
}

void ChatPainter::drawOpen(const std::string& inputText,
			   const std::vector<game::ViewChatLine>& log,
			   const bool cursorOn) {
	const std::vector<std::string> inputLines =
		world::wrapChatText(inputText, contentWidth(), measure());
	const int maxRows = world::maxHistoryRows(
		context_.height(), static_cast<int>(inputLines.size()), layout_);
	const std::vector<std::string> historyLines =
		world::visibleHistory(wrapMessages(log), maxRows);
	drawLines(inputLines, historyLines, cursorOn, 255);
}

void ChatPainter::drawPreview(const std::vector<game::ViewChatLine>& log) {
	if (log.empty()) {
		return;
	}
	// El preview muestra el PRINCIPIO del último mensaje, no el final.
	const std::vector<std::string> wrapped =
		world::wrapChatText(withSender(log.back()), contentWidth(), measure());
	const std::size_t rows = std::min<std::size_t>(
		static_cast<std::size_t>(layout_.previewRows), wrapped.size());
	const std::vector<std::string> preview(wrapped.begin(),
					      wrapped.begin() + rows);
	drawLines({}, preview, false,
		  static_cast<std::uint8_t>(layout_.previewAlpha));
}

}
