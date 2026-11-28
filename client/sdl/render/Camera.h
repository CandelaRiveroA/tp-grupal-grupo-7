#pragma once

#include "sdl/render/CameraZoom.h"
#include "sdl/render/Viewport.h"
#include "world/ScenePixelMapper.h"

namespace sdl {

// Cámara de presentación: decide qué parte del mundo entra en la ventana
// y con qué zoom. No conoce entidades, animaciones, assets ni SDL; solo
// recibe rectángulos en píxeles de mundo y produce el Viewport que consume
// el renderer. El cálculo de encuadre vive en CameraZoom.
class Camera {
public:
	Camera(const world::PixelRect& worldBounds, int viewportWidth,
	       int viewportHeight);

	// Nuevos bounds del mundo (al cambiar de escena): recalcula el zoom y
	// reencuadra. El viewport conserva su tamaño de pantalla.
	void setWorldBounds(const world::PixelRect& worldBounds);

	void follow(const world::PixelRect& target);

	const Viewport& viewport() const;

	int offsetX() const;
	int offsetY() const;

private:
	// Encuadra sin target (al construir o cambiar de escena): centra si el
	// mundo entra; si no, arranca en el origen del mundo.
	void resetFraming();

	world::PixelRect worldBounds_;
	CameraZoom framing_;
	double zoom_;
	Viewport viewport_;
};

}  // namespace sdl
