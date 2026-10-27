#include "CharacterSprite.h"

#include "sdl/render/SpriteSheet.h"

namespace sdl {

CharacterSprite::CharacterSprite(SpriteSheet& sheet, int drawnWidth,
				 int drawnHeight)
	: sheet_(sheet)
	, drawnWidth_(drawnWidth)
	, drawnHeight_(drawnHeight) {}

void CharacterSprite::draw(const world::Pose& pose,
			   const SDL2pp::Rect& logicalBox, Flip flip) const {
	if (pose.empty()) {
		return;
	}

	const world::LayerFrame& base = pose.front();
	const int pxPerCellX = drawnWidth_ / base.cols;
	const int pxPerCellY = drawnHeight_ / base.rows;

	const int bodyX = logicalBox.x + (logicalBox.w - drawnWidth_) / 2;
	const int bodyBottom = logicalBox.y + logicalBox.h;

	for (const world::LayerFrame& layer : pose) {
		const int w = layer.cols * pxPerCellX;
		const int h = layer.rows * pxPerCellY;
		const int x = bodyX;
		const int y = bodyBottom - h;
		sheet_.drawBlock(layer.frame.col, layer.frame.row, layer.cols,
				 layer.rows, SDL2pp::Rect(x, y, w, h), flip);
	}
}

}
