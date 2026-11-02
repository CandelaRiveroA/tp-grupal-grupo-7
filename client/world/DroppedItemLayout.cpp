#include "world/DroppedItemLayout.h"

namespace world {

world::PixelRect DroppedItemLayout::rectFor(
	const world::PixelRect& cellRect, const world::SpriteVisual& visual) const {
	// Tamaño en el mapa: `drawn` si lo declara, si no la mitad de la
	// celda (queda como un item apoyado en el suelo).
	const int width =
		visual.drawnWidth > 0 ? visual.drawnWidth : cellRect.width / 2;
	const int height = visual.drawnHeight > 0
		? visual.drawnHeight
		: cellRect.height / 2;
	return PixelRect{cellRect.x + (cellRect.width - width) / 2,
			 cellRect.y + cellRect.height - height + visual.offsetY,
			 width, height};
}

}  // namespace world
