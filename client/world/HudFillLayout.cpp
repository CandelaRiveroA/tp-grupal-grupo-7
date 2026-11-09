#include "world/HudFillLayout.h"

#include <algorithm>
#include <cmath>

namespace world {

HudFillLayout::HudFillLayout(const PixelBox& shell, const HudFill& fill,
			     const int scale)
	: x_(shell.x + fill.cavityX * scale)
	, y_(shell.y + fill.cavityY * scale)
	, width_(fill.cavityWidth * scale)
	, height_(fill.cavityHeight * scale) {}

PixelBox HudFillLayout::box(const double fraction) const {
	const double clamped = std::clamp(fraction, 0.0, 1.0);
	const int filled =
		static_cast<int>(std::lround(height_ * clamped));
	if (filled <= 0) {
		return PixelBox{x_, y_ + height_, width_, 0};
	}
	// Pegado abajo: lo que falta se descuenta del borde superior.
	return PixelBox{x_, y_ + height_ - filled, width_, filled};
}

}
