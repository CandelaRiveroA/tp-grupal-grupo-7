#include "world/CharacterLayout.h"

namespace world {

CharacterLayout::CharacterLayout(const PixelRect& bodyBox, int drawnWidth,
				 int drawnHeight, int referenceCols,
				 int referenceRows)
	: bodyBox_(bodyBox)
	, drawnWidth_(drawnWidth)
	, drawnHeight_(drawnHeight)
	, referenceCols_(referenceCols)
	, referenceRows_(referenceRows) {}

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

	// Con referencia, la escala es la de la criatura (igual para todos
	// sus frames); sin ella, la de la capa base de este frame.
	const bool hasReference = referenceCols_ > 0 && referenceRows_ > 0;
	const int pxPerCellX =
		drawnWidth_ / (hasReference ? referenceCols_ : baseLayer.cols);
	const int pxPerCellY =
		drawnHeight_ / (hasReference ? referenceRows_ : baseLayer.rows);

	const int w = layer.cols * pxPerCellX;
	const int h = layer.rows * pxPerCellY;
	// Con referencia el frame se centra en el cuerpo (uno más ancho
	// sobresale parejo a los dos lados).
	const int x = bodyX() + (hasReference ? (drawnWidth_ - w) / 2 : 0);
	const int y = bodyBottom() - h;
	return PixelRect{x, y, w, h};
}

}  // namespace world
