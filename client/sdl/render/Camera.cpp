#include "sdl/render/Camera.h"

#include <algorithm>

#include "sdl/render/RenderConstants.h"

namespace sdl {

Camera::Camera(const world::PixelRect& worldBounds, const int viewportWidth,
	       const int viewportHeight)
	: worldBounds_(worldBounds)
	, viewport_(worldBounds.x, worldBounds.y, viewportWidth,
		    viewportHeight) {}

void Camera::setWorldBounds(const world::PixelRect& worldBounds) {
	worldBounds_ = worldBounds;
}

void Camera::follow(const world::PixelRect& target) {
	const int targetCenterX =
		target.x + target.width / render::kAxisCenterDivisor;
	const int targetCenterY =
		target.y + target.height / render::kAxisCenterDivisor;

	const int offsetX = clampedAxisOffset(targetCenterX, worldBounds_.x,
					      worldBounds_.width,
					      viewport_.width());
	const int offsetY = clampedAxisOffset(targetCenterY, worldBounds_.y,
					      worldBounds_.height,
					      viewport_.height());
	viewport_ = Viewport(offsetX, offsetY, viewport_.width(),
			     viewport_.height());
}

const Viewport& Camera::viewport() const { return viewport_; }

int Camera::offsetX() const { return viewport_.offsetX(); }

int Camera::offsetY() const { return viewport_.offsetY(); }

int Camera::clampedAxisOffset(const int targetCenter, const int worldOrigin,
			      const int worldSize,
			      const int viewportSize) const {
	const int desired =
		targetCenter - viewportSize / render::kAxisCenterDivisor;
	const int maxOffset =
		std::max(worldSize - viewportSize, render::kMinCameraOffset);
	return std::clamp(desired, worldOrigin, worldOrigin + maxOffset);
}

}  // namespace sdl
