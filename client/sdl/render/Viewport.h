#pragma once

#include "world/ScenePixelMapper.h"

namespace sdl {

// Frontera entre cámara y render: representa la ventana visible en píxeles
// de pantalla y permite traducir/consultar rectángulos de mundo.
class Viewport {
public:
	Viewport(int offsetX, int offsetY, int width, int height);

	int offsetX() const;
	int offsetY() const;
	int width() const;
	int height() const;

	world::PixelRect screenRect() const;
	world::PixelRect visibleWorldRect() const;
	world::PixelRect toScreen(const world::PixelRect& worldRect) const;

	// Inversa de toScreen para puntos: px de pantalla -> px de mundo.
	// La única dueña de esta traducción (POLK); el cursor y el pick la
	// usan para saber qué hay bajo el puntero.
	world::PixelPoint toWorld(const world::PixelPoint& screenPoint) const;

	bool intersects(const world::PixelRect& screenRect) const;

private:
	int offsetX_;
	int offsetY_;
	int width_;
	int height_;
};

}  // namespace sdl
