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

int ToolOverlayLayout::frameIndexAt(const int frameTicks,
				    const std::size_t frameCount,
				    const int phase) const {
	if (frameCount == 0) {
		return -1;
	}
	const int frameIndex =
		(frameTicks > 0) ? phase / frameTicks : 0;
	const int lastIndex = static_cast<int>(frameCount) - 1;
	return std::clamp(frameIndex, 0, lastIndex);
}

const ToolOverlayFrame* ToolOverlayLayout::frameAt(
	const AnimatedOverlay& action, const int phase) const {
	const int index =
		frameIndexAt(action.frameTicks, action.frames.size(), phase);
	if (index < 0) {
		return nullptr;
	}
	return &action.frames[static_cast<std::size_t>(index)];
}

const HeldOverlayFrame* ToolOverlayLayout::frameAt(
	const HeldContentOverlay& overlay, const int phase) const {
	const int index = frameIndexAt(overlay.frameTicks,
				       overlay.frames.size(), phase);
	if (index < 0) {
		return nullptr;
	}
	return &overlay.frames[static_cast<std::size_t>(index)];
}

PixelRect ToolOverlayLayout::layerRect(const OverlayFrame& frame,
				       const OverlayLayer& layer) const {
	// El factor de píxeles por celda lo pone CharacterLayout desde la
	// capa base del pose (el cuerpo): una capa 1x1 mide una celda
	// aunque el cuerpo sea 1x2. Sobre ese tamaño se aplican la escala
	// y el anclaje del frame. Una capa inválida devuelve el rect
	// vacío de CharacterLayout, sin dividir por cero.
	const PixelRect cell = body_.layerRect(layer.layer, base_);

	// Tamaño de UNA celda del layer (en px): la base del desplazamiento
	// en celdas y de la normalización. Con cellSpan > 0 el patrón se
	// mete en UNA celda lógica (cada capa = celda / cellSpan), así el
	// chorro no crece al crecer el grupo. Sin cellSpan, cada capa mide
	// una celda entera (el caso histórico de la herramienta).
	const int layerCols = (layer.layer.cols > 0) ? layer.layer.cols : 1;
	const int layerRows = (layer.layer.rows > 0) ? layer.layer.rows : 1;
	const int baseCellW = cell.width / layerCols;
	const int baseCellH = cell.height / layerRows;
	const int cellW = (frame.cellSpanX > 0) ? baseCellW / frame.cellSpanX
						: baseCellW;
	const int cellH = (frame.cellSpanY > 0) ? baseCellH / frame.cellSpanY
						: baseCellH;

	const int width =
		static_cast<int>(cellW * layerCols * frame.scale);
	const int height =
		static_cast<int>(cellH * layerRows * frame.scale);
	const int x = body_.bodyX() + frame.offsetX + layer.gridOffsetX * cellW;
	const int y = anchorBaseY(frame.anchor, height) +
		      catalog_.offsetFor(frame.anchor) + frame.offsetY +
		      layer.gridOffsetY * cellH;
	return PixelRect{x, y, width, height};
}

}  // namespace world
