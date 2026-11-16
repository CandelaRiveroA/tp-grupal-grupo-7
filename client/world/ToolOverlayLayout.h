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

	// Frame de la animación para la fase recibida, de la pasada pedida
	// (detrás/delante del cuerpo). nullptr si la acción no tiene frames
	// dibujables o si el frame de esa fase pertenece a la otra pasada.
	// Toma AnimatedOverlay: sirve tanto para la herramienta como para su
	// efecto (el chorro), que comparten el mismo dato de animación.
	const ToolOverlayFrame* frameAt(const AnimatedOverlay& action,
					int phase, OverlayPass pass) const;

	// Frame del contenido sostenido para la fase recibida (misma ley de
	// avance que las herramientas). nullptr si no hay frames o si el de
	// esa fase pertenece a la otra pasada.
	const HeldOverlayFrame* frameAt(const HeldContentOverlay& overlay,
					int phase, OverlayPass pass) const;

	// Rectángulo de una capa del overlay. Aplica el offset EN CELDAS de
	// la capa (distribuye las gotas del chorro en 2D). Si la capa es
	// inválida devuelve un rect vacío, sin dividir por cero.
	PixelRect layerRect(const OverlayFrame& frame,
			    const OverlayLayer& layer) const;

private:
	using AnchorBaseMethod = int (ToolOverlayLayout::*)(int frameHeight) const;

	int topBase(int frameHeight) const;
	int bodyBase(int frameHeight) const;
	int bottomBase(int frameHeight) const;

	int anchorBaseY(FrameAnchor anchor, int frameHeight) const;

	// Índice del frame que corresponde a una fase (compartido por las
	// dos clases de overlay).
	int frameIndexAt(int frameTicks, std::size_t frameCount,
			 int phase) const;

	// Un frame pertenece a la pasada pedida según su flag `behind`
	// (compartido por las dos clases de overlay).
	bool inPass(const OverlayFrame& frame, OverlayPass pass) const;

	CharacterLayout body_;
	// Capa base del pose (el cuerpo): de ella sale el factor de píxeles
	// por celda, igual que CharacterLayout. Así una capa 1x1 del
	// overlay (el huevo) mide una celda aunque el cuerpo sea 1x2.
	LayerFrame base_;
	const ToolOverlayCatalog& catalog_;
	std::map<FrameAnchor, AnchorBaseMethod> anchorBases_;
};

}  // namespace world
