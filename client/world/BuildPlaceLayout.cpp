#include "world/BuildPlaceLayout.h"

namespace world {

PixelRect BuildPlaceLayout::rectFor(const PixelRect& cellRect,
				    const BuildPlaceVisual& visual) const {
	const int width = static_cast<int>(cellRect.width * visual.scale);
	const int height = static_cast<int>(cellRect.height * visual.scale);
	return PixelRect{cellRect.x + (cellRect.width - width) / 2,
			 cellRect.y + (cellRect.height - height) / 2, width,
			 height};
}

}
