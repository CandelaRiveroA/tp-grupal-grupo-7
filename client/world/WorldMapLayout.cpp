#include "world/WorldMapLayout.h"

#include <algorithm>

namespace world {

WorldMapLayout::WorldMapLayout(const int screenW, const int screenH,
			       const WorldMapConfig& config,
			       const PixelSize& mapTextureSize)
	: modal_(screenW, screenH, config.modal,
		 config.titleTopPad + config.titleHeight + config.mapTopGap,
		 config.modal.framePad)
	, mapBorder_(config.mapBorder)
	, pinSize_(config.pinSize)
	, pinBorder_(config.pinBorder) {
	const PixelBox& box = modal_.panel();
	titleBox_ = PixelBox{
		box.x + config.modal.framePad,
		box.y + config.titleTopPad,
		box.width - 2 * config.modal.framePad,
		config.titleHeight,
	};
	mapRect_ = modal_.innerContentRect().fitCentered(mapTextureSize);
}

const PagedModalLayout& WorldMapLayout::modal() const { return modal_; }

const PixelBox& WorldMapLayout::titleBox() const { return titleBox_; }

const PixelBox& WorldMapLayout::mapRect() const { return mapRect_; }

PixelBox WorldMapLayout::mapBorderBox() const {
	return mapRect_.outset(mapBorder_);
}

PixelBox WorldMapLayout::pinOuterBox(const float normX,
				     const float normY) const {
	const int centerX =
		mapRect_.x +
		static_cast<int>(normX * static_cast<float>(mapRect_.width));
	const int centerY =
		mapRect_.y +
		static_cast<int>(normY * static_cast<float>(mapRect_.height));
	return PixelBox{
		centerX - pinSize_ / 2,
		centerY - pinSize_ / 2,
		pinSize_,
		pinSize_,
	};
}

PixelBox WorldMapLayout::pinInnerBox(const PixelBox& outer) const {
	const PixelBox inner = outer.inset(pinBorder_);
	return PixelBox{
		inner.x,
		inner.y,
		std::max(1, inner.width),
		std::max(1, inner.height),
	};
}

bool WorldMapLayout::hitsSwitchArrow(const int x, const int y) const {
	return modal_.hitsAnyArrow(x, y);
}

}  // namespace world
