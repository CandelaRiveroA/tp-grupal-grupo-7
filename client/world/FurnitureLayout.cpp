#include "world/FurnitureLayout.h"

namespace world {

FurnitureLayout::FurnitureLayout(const ScenePixelMapper& mapper)
	: mapper_(mapper) {}

PixelRect FurnitureLayout::box(const SpriteVisual& visual,
			       const GridCell& base) const {
	const GridCell topLeft{base.column, base.row - visual.rows + 1};
	return mapper_.toPixels(topLeft, GridSpan{visual.cols, visual.rows});
}

}
