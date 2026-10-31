#include "CharacterSprite.h"

#include "sdl/render/SpriteSheet.h"
#include "world/CharacterLayout.h"

namespace sdl {

CharacterSprite::CharacterSprite(SpriteSheet& sheet, int drawnWidth,
				 int drawnHeight)
	: sheet_(sheet)
	, drawnWidth_(drawnWidth)
	, drawnHeight_(drawnHeight) {}

void CharacterSprite::draw(const world::Pose& pose,
			   const SDL2pp::Rect& logicalBox, Flip flip) {
	if (pose.empty()) {
		return;
	}

	const world::CharacterLayout layout(
		world::PixelRect{logicalBox.x, logicalBox.y, logicalBox.w,
				 logicalBox.h},
		drawnWidth_, drawnHeight_);

	for (const world::LayerFrame& layer : pose) {
		const world::PixelRect& rect = layout.layerRect(layer);
		sheet_.drawBlock(layer.frame.col, layer.frame.row, layer.cols,
				 layer.rows,
				 SDL2pp::Rect(rect.x, rect.y, rect.width,
					      rect.height),
				 flip);
	}
}

}
