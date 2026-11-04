#pragma once

#include "sdl/render/Viewport.h"
#include "world/ScenePixelMapper.h"

namespace sdl {

// Cámara de presentación: decide qué parte del mundo entra en la ventana.
// No conoce entidades, animaciones, assets ni SDL; solo recibe rectángulos
// en píxeles de mundo y produce el Viewport que consume el renderer.
class Camera {
public:
	Camera(const world::PixelRect& worldBounds, int viewportWidth,
	       int viewportHeight);

	void follow(const world::PixelRect& target);

	const Viewport& viewport() const;

	int offsetX() const;
	int offsetY() const;

private:
	int clampedAxisOffset(int targetCenter, int worldOrigin, int worldSize,
			      int viewportSize) const;

	world::PixelRect worldBounds_;
	Viewport viewport_;
};

}  // namespace sdl
