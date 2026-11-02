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

	PixelRect worldBounds(const GridSpan& grid) const;

private:
	int cellPixelSize_;
};

}
