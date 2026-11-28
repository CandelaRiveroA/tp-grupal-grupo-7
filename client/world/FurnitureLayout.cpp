#include "world/FurnitureLayout.h"

namespace world {

FurnitureLayout::FurnitureLayout(const ScenePixelMapper& mapper)
	: mapper_(mapper) {}

PixelRect FurnitureLayout::box(const SpriteVisual& visual,
			       const GridCell& base) const {
	const GridCell topLeft{base.column, base.row - visual.rows + 1};
	return mapper_.toPixels(topLeft, GridSpan{visual.cols, visual.rows});
}

PixelRect FurnitureLayout::pushed(const PixelRect& box, const GridCell& toward,
				  const double push) const {
	PixelRect rect = box;
	const double shift = push * mapper_.cellPixelSize();
	rect.x += static_cast<int>(toward.column * shift);
	rect.y += static_cast<int>(toward.row * shift);
	return rect;
}

}
