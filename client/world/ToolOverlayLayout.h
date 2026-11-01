#pragma once

#include <map>

#include "world/CharacterLayout.h"
#include "world/ToolOverlay.h"

namespace world {

// Geometría pura del overlay de herramienta. No conoce SDL ni assets: elige
// el frame de la animación y calcula el rectángulo de cada capa respecto del
// cuerpo, aplicando anclaje, offsets y escala. El pintado queda en
// sdl::ToolOverlayPainter.
class ToolOverlayLayout {
public:
	ToolOverlayLayout(const CharacterLayout& body,
			  const ToolOverlayCatalog& catalog,
			  const LayerFrame& baseLayer);

	// Frame de la animación para la fase recibida. nullptr si la acción
	// no tiene frames dibujables.
	const ToolOverlayFrame* frameAt(const ToolOverlayAction& action,
					int phase) const;

	// Rectángulo de una capa del overlay. Si la capa es inválida
	// devuelve un rect vacío, sin dividir por cero.
	PixelRect layerRect(const ToolOverlayFrame& frame,
			    const LayerFrame& layer) const;

private:
	using AnchorBaseMethod = int (ToolOverlayLayout::*)(int frameHeight) const;

	int topBase(int frameHeight) const;
	int bodyBase(int frameHeight) const;
	int bottomBase(int frameHeight) const;

	int anchorBaseY(FrameAnchor anchor, int frameHeight) const;

	CharacterLayout body_;
	// Capa base del pose (el cuerpo): de ella sale el factor de píxeles
	// por celda, igual que CharacterLayout. Así una capa 1x1 del
	// overlay (el huevo) mide una celda aunque el cuerpo sea 1x2.
	LayerFrame base_;
	const ToolOverlayCatalog& catalog_;
	std::map<FrameAnchor, AnchorBaseMethod> anchorBases_;
};

}  // namespace world
