#include "world/ElementLayout.h"

namespace world {

ElementLayout::ElementLayout(const TileSetDef& tileset,
			     const ElementPlacement& element,
			     const PixelRect& area)
	: tileset_(tileset), element_(element), area_(area) {
	boundsByMode_ = {
		{DrawMode::Single,
		 PixelRect{area_.x, area_.y, pieceWidth(), pieceHeight()}},
		{DrawMode::Tile, area_},
		{DrawMode::HorizontalStrip,
		 PixelRect{area_.x, area_.y, area_.width, pieceHeight()}},
	};
}

const PixelRect& ElementLayout::area() const { return area_; }

const ElementPlacement& ElementLayout::element() const { return element_; }

DrawMode ElementLayout::mode() const { return element_.draw.mode; }

int ElementLayout::pieceWidth() const {
	return tileset_.cellWidth * element_.pieceSize.columns *
	       element_.draw.scaleX;
}

int ElementLayout::pieceHeight() const {
	return tileset_.cellHeight * element_.pieceSize.rows *
	       element_.draw.scaleY;
}

PixelRect ElementLayout::bounds() const {
	return boundsByMode_.at(element_.draw.mode);
}

}  // namespace world
