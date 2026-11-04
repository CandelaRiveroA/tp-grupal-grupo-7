#include "world/HeldItemLayout.h"

namespace world {

HeldItemLayout::HeldItemLayout(const PixelRect& bodyBox, int drawnWidth,
			       int drawnHeight, int itemWidth,
			       int itemHeight, int offsetY)
	: body_(bodyBox, drawnWidth, drawnHeight)
	, itemWidth_(itemWidth)
	, itemHeight_(itemHeight)
	, offsetY_(offsetY) {}

PixelRect HeldItemLayout::rect() const {
	// Centrado sobre el cuerpo, con el borde inferior del ítem a
	// offsetY px sobre la cabeza (subir el offset lo baja).
	const int x = body_.bodyX() + (body_.drawnWidth() - itemWidth_) / 2;
	const int y = body_.bodyTop() - offsetY_ - itemHeight_;
	return PixelRect{x, y, itemWidth_, itemHeight_};
}

}
