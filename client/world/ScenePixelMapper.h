#pragma once

#include "world/Grid.h"

namespace world {

struct PixelRect {
	int x;
	int y;
	int width;
	int height;
};

// Punto en píxeles (de mundo o de pantalla, según el contexto).
struct PixelPoint {
	int x = 0;
	int y = 0;
};

class ScenePixelMapper {
public:
	explicit ScenePixelMapper(int cellPixelSize);

	int cellPixelSize() const;

	PixelRect toPixels(const GridCell& at, const GridSpan& span) const;

	// Inversa de toPixels: la celda que contiene un px de mundo (los
	// puntos fuera del mundo caen en la celda que los toca por su
	// signo). Es la única conversión px -> celda del proyecto: la
	// usan el pick del click y el validador de alcance.
	GridCell toCell(const PixelPoint& point) const;

	PixelRect worldBounds(const GridSpan& grid) const;

private:
	// Celda de un eje para un px (división hacia -infinito: los
	// negativos no redondean hacia cero).
	int cellIndex(int pixels) const;

	int cellPixelSize_;
};

}
