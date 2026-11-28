#include "world/PixelBox.h"

#include <algorithm>

namespace world {

bool PixelBox::contains(const int px, const int py) const {
	return px >= x && px < x + width && py >= y && py < y + height;
}

PixelBox PixelBox::inset(const int px) const {
	if (px <= 0) {
		return *this;
	}
	return PixelBox{
		x + px,
		y + px,
		std::max(0, width - 2 * px),
		std::max(0, height - 2 * px),
	};
}

PixelBox PixelBox::outset(const int px) const {
	if (px <= 0) {
		return *this;
	}
	return PixelBox{
		x - px,
		y - px,
		width + 2 * px,
		height + 2 * px,
	};
}

PixelBox PixelBox::fitCentered(const PixelSize& content) const {
	if (content.width <= 0 || content.height <= 0 || width <= 0 ||
	    height <= 0) {
		return *this;
	}
	const double scale = std::min(
		static_cast<double>(width) / static_cast<double>(content.width),
		static_cast<double>(height) /
			static_cast<double>(content.height));
	const int fitW = std::max(1, static_cast<int>(content.width * scale));
	const int fitH = std::max(1, static_cast<int>(content.height * scale));
	return PixelBox{
		x + (width - fitW) / 2,
		y + (height - fitH) / 2,
		fitW,
		fitH,
	};
}

}  // namespace world
