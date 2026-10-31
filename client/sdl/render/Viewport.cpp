#include "sdl/render/Viewport.h"

#include <algorithm>

#include "sdl/render/RenderConstants.h"

namespace sdl {

Viewport::Viewport(const int offsetX, const int offsetY, const int width,
		    const int height)
	: offsetX_(offsetX)
	, offsetY_(offsetY)
	, width_(width)
	, height_(height) {}

int Viewport::offsetX() const { return offsetX_; }

int Viewport::offsetY() const { return offsetY_; }

int Viewport::width() const { return width_; }

int Viewport::height() const { return height_; }

world::PixelRect Viewport::screenRect() const {
	return world::PixelRect{render::kScreenOrigin, render::kScreenOrigin,
				width_, height_};
}

world::PixelRect Viewport::visibleWorldRect() const {
	return world::PixelRect{offsetX_, offsetY_, width_, height_};
}

world::PixelRect Viewport::toScreen(const world::PixelRect& worldRect) const {
	return world::PixelRect{worldRect.x - offsetX_,
				worldRect.y - offsetY_, worldRect.width,
				worldRect.height};
}

bool Viewport::intersects(const world::PixelRect& screenRect) const {
	if (screenRect.width <= 0 || screenRect.height <= 0) {
		return false;
	}

	const int left = std::max(screenRect.x, render::kScreenOrigin);
	const int top = std::max(screenRect.y, render::kScreenOrigin);
	const int right = std::min(screenRect.x + screenRect.width, width_);
	const int bottom = std::min(screenRect.y + screenRect.height, height_);
	return left < right && top < bottom;
}

}  // namespace sdl
