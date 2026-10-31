#include "world/ToolOverlayLayout.h"

#include <algorithm>

namespace world {

ToolOverlayLayout::ToolOverlayLayout(const CharacterLayout& body,
				     const ToolOverlayCatalog& catalog)
	: body_(body)
	, catalog_(catalog)
	, anchorBases_{
		  {FrameAnchor::Top, &ToolOverlayLayout::topBase},
		  {FrameAnchor::Body, &ToolOverlayLayout::bodyBase},
		  {FrameAnchor::Bottom, &ToolOverlayLayout::bottomBase},
	  } {}

int ToolOverlayLayout::topBase(const int frameHeight) const {
	return body_.bodyTop() - frameHeight;
}

int ToolOverlayLayout::bodyBase(int) const { return body_.bodyTop(); }

int ToolOverlayLayout::bottomBase(const int frameHeight) const {
	return body_.bodyBottom() - frameHeight;
}

int ToolOverlayLayout::anchorBaseY(const FrameAnchor anchor,
				   const int frameHeight) const {
	return (this->*anchorBases_.at(anchor))(frameHeight);
}

const ToolOverlayFrame* ToolOverlayLayout::frameAt(
	const ToolOverlayAction& action, const int phase) const {
	if (action.frames.empty()) {
		return nullptr;
	}

	const int frameIndex =
		(action.frameTicks > 0) ? phase / action.frameTicks : 0;
	const int lastIndex =
		static_cast<int>(action.frames.size()) - 1;
	const int clampedIndex = std::clamp(frameIndex, 0, lastIndex);
	return &action.frames[static_cast<std::size_t>(clampedIndex)];
}

PixelRect ToolOverlayLayout::layerRect(const ToolOverlayFrame& frame,
				       const LayerFrame& layer) const {
	if (layer.cols <= 0 || layer.rows <= 0) {
		return PixelRect{body_.bodyX(), body_.bodyBottom(), 0, 0};
	}

	const int pxPerCellX = body_.drawnWidth() / layer.cols;
	const int pxPerCellY = body_.drawnHeight() / layer.rows;
	const int width =
		static_cast<int>(layer.cols * pxPerCellX * frame.scale);
	const int height =
		static_cast<int>(layer.rows * pxPerCellY * frame.scale);
	const int x = body_.bodyX() + frame.offsetX;
	const int y = anchorBaseY(frame.anchor, height) +
		      catalog_.offsetFor(frame.anchor) + frame.offsetY;
	return PixelRect{x, y, width, height};
}

}  // namespace world
