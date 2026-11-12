#include "world/ChatPanel.h"

#include <algorithm>

namespace world {

namespace {

int inputTop(const int windowHeight, const ChatLayout& layout) {
	return static_cast<int>(static_cast<double>(windowHeight) *
				layout.inputFraction);
}

// 0 renglones = sin caja (el preview) el alto es 0, sin padding.
struct InputBox {
	int rows = 0;
	int height = 0;
};

InputBox inputBox(const int windowHeight, const int inputRows,
		  const ChatLayout& layout) {
	if (inputRows <= 0) {
		return InputBox{};
	}
	const int room =
		windowHeight - layout.marginBottom - inputTop(windowHeight, layout);
	const int maxRows =
		std::max(1, (room - 2 * layout.textPad) / layout.lineHeight());
	const int rows = std::min(inputRows, maxRows);
	return InputBox{rows, rows * layout.lineHeight() + 2 * layout.textPad};
}

}

int chatBoxWidth(const int windowWidth, const ChatLayout& layout) {
	const int maxWidth = std::max(0, windowWidth - 2 * layout.marginX);
	const int fraction = static_cast<int>(static_cast<double>(windowWidth) * layout.widthFraction);
	return std::min(fraction, maxWidth);
}

int maxHistoryRows(const int windowHeight, const int inputRows,
		   const ChatLayout& layout) {
	// Solo renglones enteros: si no cabe ninguno, no hay caja.
	const int room =
		inputTop(windowHeight, layout) - inputBox(windowHeight, inputRows, layout).height;
	const int pad = layout.textPad;
	return room >= 2 * pad ? (room - 2 * pad) / layout.lineHeight() : 0;
}

ChatPanel chatPanels(const int windowWidth, const int windowHeight,
		     const int inputRows, const int historyRows,
		     const ChatLayout& layout) {
	const int lineH = layout.lineHeight();
	const int pad = layout.textPad;
	const int boxWidth = chatBoxWidth(windowWidth, layout);
	const InputBox typing = inputBox(windowHeight, inputRows, layout);
	const int boxX = layout.marginX;
	const int typingTop = inputTop(windowHeight, layout);

	const int maxRows = maxHistoryRows(windowHeight, inputRows, layout);
	const int visibleRows = std::min(std::max(0, historyRows), maxRows);
	const int historyHeight = visibleRows > 0 ? visibleRows * lineH + 2 * pad : 0;

	PixelBox history;
	if (visibleRows > 0) {
		history = PixelBox{boxX, typingTop - historyHeight, boxWidth, historyHeight};
	}

	PixelBox input;
	if (typing.rows > 0) {
		input = PixelBox{boxX, typingTop, boxWidth, typing.height};
	}

	return ChatPanel{history, input, typing.rows};
}

}