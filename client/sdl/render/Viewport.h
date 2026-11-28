#pragma once

#include "world/ScenePixelMapper.h"

namespace sdl {

// Frontera entre cámara y render: representa la ventana visible en píxeles
// de pantalla y permite traducir/consultar rectángulos de mundo. Es la
// única dueña del transform mundo <-> pantalla (offset + zoom).
class Viewport {
public:
	Viewport(int offsetX, int offsetY, int width, int height,
		 double zoom = 1.0);

	int offsetX() const;
	int offsetY() const;
	int width() const;
	int height() const;
	double zoom() const;

	world::PixelRect screenRect() const;
	world::PixelRect visibleWorldRect() const;
	world::PixelRect toScreen(const world::PixelRect& worldRect) const;

	// Inversa de toScreen para puntos: px de pantalla -> px de mundo.
	// La única dueña de esta traducción (POLK); el cursor y el pick la
	// usan para saber qué hay bajo el puntero.
	world::PixelPoint toWorld(const world::PixelPoint& screenPoint) const;

	bool intersects(const world::PixelRect& screenRect) const;

private:
	// Borde de pantalla de una coordenada de mundo. Se redondea cada
	// borde (no el tamaño) para que dos rectángulos contiguos compartan
	// exactamente el mismo píxel de pantalla y no queden costuras.
	int screenEdge(int worldCoordinate, int offset) const;

	int offsetX_;
	int offsetY_;
	int width_;
	int height_;
	double zoom_;
};

}  // namespace sdl
