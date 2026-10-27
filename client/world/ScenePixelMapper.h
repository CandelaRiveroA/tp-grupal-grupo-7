#pragma once

#include "world/Grid.h"

namespace world {

struct PixelRect {
	int x;
	int y;
	int width;
	int height;
};

class ScenePixelMapper {
public:
	explicit ScenePixelMapper(int cellPixelSize);

	int cellPixelSize() const;

	PixelRect toPixels(const GridCell& at, const GridSpan& span) const;

private:
	int cellPixelSize_;
};

}
