#include "world/ChatPanel.h"

#include <algorithm>

namespace world {

namespace {

int inputTop(const int windowHeight, const ChatLayout& layout) {
	return static_cast<int>(static_cast<double>(windowHeight) *
				layout.inputFraction);
}

}

int chatBoxWidth(const int windowWidth, const ChatLayout& layout) {
	const int maxWidth = std::max(0, windowWidth - 2 * layout.marginX);
	const int fraction = static_cast<int>(static_cast<double>(windowWidth) * layout.widthFraction);
	return std::min(fraction, maxWidth);
}

ChatPanel chatPanels(const int windowWidth, const int windowHeight,
		     const int inputSurfaceHeight,
		     const std::vector<int>& historySurfaceHeights,
		     const ChatLayout& layout) {
	const int pad = layout.textPad;
	const int boxWidth = chatBoxWidth(windowWidth, layout);
	const int typingTop = inputTop(windowHeight, layout);

	// Crece hacia abajo el inputBox
	PixelBox input;
	if (inputSurfaceHeight > 0) {
		const int room =
			windowHeight - layout.marginBottom - typingTop;
		const int height =
			std::min(inputSurfaceHeight + 2 * pad, room);
		input = PixelBox{layout.marginX, typingTop, boxWidth, height};
	}

	// El history crece hacia arriba
	PixelBox history;
	int used = 0;
	for (const int height: historySurfaceHeights) {
		used += height;
	}
	if (used > 0) {
		const int height = std::min(used + 2 * pad, typingTop);
		history = PixelBox{layout.marginX, typingTop - height,
				   boxWidth, height};
	}

	return ChatPanel{history, input};
}

}
