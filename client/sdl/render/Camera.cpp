#include "sdl/render/Camera.h"

#include "sdl/render/RenderConstants.h"

namespace sdl {

Camera::Camera(const world::PixelRect& worldBounds, const int viewportWidth,
	       const int viewportHeight)
	: worldBounds_(worldBounds)
	, framing_()
	, zoom_(framing_.fitZoom(worldBounds_, viewportWidth, viewportHeight))
	, viewport_(worldBounds.x, worldBounds.y, viewportWidth, viewportHeight,
		    zoom_) {
	resetFraming();
}

void Camera::setWorldBounds(const world::PixelRect& worldBounds) {
	worldBounds_ = worldBounds;
	zoom_ = framing_.fitZoom(worldBounds_, viewport_.width(),
				 viewport_.height());
	resetFraming();
}

void Camera::follow(const world::PixelRect& target) {
	const int targetCenterX =
		target.x + target.width / render::kAxisCenterDivisor;
	const int targetCenterY =
		target.y + target.height / render::kAxisCenterDivisor;

	const int visibleWidth =
		framing_.visibleSize(viewport_.width(), zoom_);
	const int visibleHeight =
		framing_.visibleSize(viewport_.height(), zoom_);
	// Solo el eje Y recorta la banda del HUD: el mundo centrado queda
	// sobre el inventario, no detrás.
	const int playHeight =
		framing_.playVisibleSize(viewport_.height(), zoom_);

	viewport_ = Viewport(
		framing_.axisOffset(targetCenterX, worldBounds_.x,
				    worldBounds_.width, visibleWidth,
				    visibleWidth),
		framing_.axisOffset(targetCenterY, worldBounds_.y,
				    worldBounds_.height, visibleHeight,
				    playHeight),
		viewport_.width(), viewport_.height(), zoom_);
}

const Viewport& Camera::viewport() const { return viewport_; }

int Camera::offsetX() const { return viewport_.offsetX(); }

int Camera::offsetY() const { return viewport_.offsetY(); }

void Camera::resetFraming() {
	const int visibleWidth =
		framing_.visibleSize(viewport_.width(), zoom_);
	const int visibleHeight =
		framing_.visibleSize(viewport_.height(), zoom_);
	const int playHeight =
		framing_.playVisibleSize(viewport_.height(), zoom_);

	viewport_ = Viewport(
		framing_.axisOffset(worldBounds_.x, worldBounds_.x,
				    worldBounds_.width, visibleWidth,
				    visibleWidth),
		framing_.axisOffset(worldBounds_.y, worldBounds_.y,
				    worldBounds_.height, visibleHeight,
				    playHeight),
		viewport_.width(), viewport_.height(), zoom_);
}

}  // namespace sdl
