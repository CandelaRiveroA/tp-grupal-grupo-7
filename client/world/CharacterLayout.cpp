#include "world/CharacterLayout.h"

namespace world {

CharacterLayout::CharacterLayout(const PixelRect& bodyBox, int drawnWidth,
				 int drawnHeight)
	: bodyBox_(bodyBox)
	, drawnWidth_(drawnWidth)
	, drawnHeight_(drawnHeight) {}

int CharacterLayout::bodyX() const {
	return bodyBox_.x + (bodyBox_.width - drawnWidth_) / 2;
}

int CharacterLayout::bodyTop() const { return bodyBottom() - drawnHeight_; }

int CharacterLayout::bodyBottom() const {
	return bodyBox_.y + bodyBox_.height;
}

int CharacterLayout::drawnWidth() const { return drawnWidth_; }

int CharacterLayout::drawnHeight() const { return drawnHeight_; }

PixelRect CharacterLayout::layerRect(const LayerFrame& layer,
				     const LayerFrame& baseLayer) const {
	if (layer.cols <= 0 || layer.rows <= 0 || baseLayer.cols <= 0 ||
	    baseLayer.rows <= 0) {
		return PixelRect{bodyX(), bodyBottom(), 0, 0};
	}

	const int pxPerCellX = drawnWidth_ / baseLayer.cols;
	const int pxPerCellY = drawnHeight_ / baseLayer.rows;

	const int w = layer.cols * pxPerCellX;
	const int h = layer.rows * pxPerCellY;
	const int x = bodyX();
	const int y = bodyBottom() - h;
	return PixelRect{x, y, w, h};
}

}  // namespace world
