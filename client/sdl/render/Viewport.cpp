#include "sdl/render/Viewport.h"

#include <algorithm>
#include <cmath>

#include "sdl/render/RenderConstants.h"

namespace sdl {

Viewport::Viewport(const int offsetX, const int offsetY, const int width,
		   const int height, const double zoom)
	: offsetX_(offsetX)
	, offsetY_(offsetY)
	, width_(width)
	, height_(height)
	, zoom_(zoom > 0.0 ? zoom : 1.0) {}

int Viewport::offsetX() const { return offsetX_; }

int Viewport::offsetY() const { return offsetY_; }

int Viewport::width() const { return width_; }

int Viewport::height() const { return height_; }

double Viewport::zoom() const { return zoom_; }

world::PixelRect Viewport::screenRect() const {
	return world::PixelRect{render::kScreenOrigin, render::kScreenOrigin,
				width_, height_};
}

world::PixelRect Viewport::visibleWorldRect() const {
	return world::PixelRect{
		offsetX_, offsetY_,
		static_cast<int>(std::ceil(width_ / zoom_)),
		static_cast<int>(std::ceil(height_ / zoom_))};
}

world::PixelRect Viewport::toScreen(const world::PixelRect& worldRect) const {
	const int left = screenEdge(worldRect.x, offsetX_);
	const int top = screenEdge(worldRect.y, offsetY_);
	const int right = screenEdge(worldRect.x + worldRect.width, offsetX_);
	const int bottom = screenEdge(worldRect.y + worldRect.height, offsetY_);
	return world::PixelRect{left, top, right - left, bottom - top};
}

world::PixelPoint Viewport::toWorld(
	const world::PixelPoint& screenPoint) const {
	return world::PixelPoint{
		offsetX_ + static_cast<int>(std::floor(screenPoint.x / zoom_)),
		offsetY_ + static_cast<int>(std::floor(screenPoint.y / zoom_))};
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

int Viewport::screenEdge(const int worldCoordinate, const int offset) const {
	return static_cast<int>(
		std::lround((worldCoordinate - offset) * zoom_));
}

}  // namespace sdl
