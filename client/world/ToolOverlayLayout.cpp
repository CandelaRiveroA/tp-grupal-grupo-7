#include "world/ToolOverlayLayout.h"

#include <algorithm>

namespace world {

ToolOverlayLayout::ToolOverlayLayout(const CharacterLayout& body,
				     const ToolOverlayCatalog& catalog,
				     const LayerFrame& baseLayer)
	: body_(body)
	, base_(baseLayer)
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
	// El factor de píxeles por celda lo pone CharacterLayout desde la
	// capa base del pose (el cuerpo): una capa 1x1 mide una celda
	// aunque el cuerpo sea 1x2. Sobre ese tamaño se aplican la escala
	// y el anclaje del frame. Una capa inválida devuelve el rect
	// vacío de CharacterLayout, sin dividir por cero.
	const PixelRect cell = body_.layerRect(layer, base_);
	const int width = static_cast<int>(cell.width * frame.scale);
	const int height = static_cast<int>(cell.height * frame.scale);
	const int x = body_.bodyX() + frame.offsetX;
	const int y = anchorBaseY(frame.anchor, height) +
		      catalog_.offsetFor(frame.anchor) + frame.offsetY;
	return PixelRect{x, y, width, height};
}

}  // namespace world
