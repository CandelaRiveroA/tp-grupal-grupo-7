#include "world/DialogLayout.h"

namespace world {

DialogLayout::DialogLayout(const DialogData& data) : data_(data) {}

PixelRect DialogLayout::dialogRect(const PixelRect& anchorBox) const {
	return PixelRect{anchorBox.x + data_.anchorOffset.x,
			 anchorBox.y + data_.anchorOffset.y,
			 data_.animation.sprite.displayWidth(),
			 data_.animation.sprite.displayHeight()};
}

PixelRect DialogLayout::contentRect(const PixelRect& dialogBox) const {
	const int width = data_.content->displayWidth();
	const int height = data_.content->displayHeight();
	return PixelRect{dialogBox.x + (dialogBox.width - width) / 2 +
				 data_.contentOffset.x,
			 dialogBox.y + (dialogBox.height - height) / 2 +
				 data_.contentOffset.y,
			 width, height};
}

}  // namespace world
