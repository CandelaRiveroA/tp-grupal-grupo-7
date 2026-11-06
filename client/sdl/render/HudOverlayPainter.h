#pragma once

#include <map>
#include <string>

#include "sdl/render/SheetCache.h"
#include "world/AnchorLayout.h"
#include "world/HudOverlayCatalog.h"

namespace sdl {
class SdlContext;
class TextRenderer;
}

namespace sdl {

// Dibuja los overlays de HUD fijos (cosas como el HUD de tiempo). Cada
// overlay es una región de una hoja anclada a un borde/centro de la
// ventana. La geometría (dónde cae cada caja) la resuelve
// world::AnchorLayout; acá solo se blitea (convención §9: el render no
// reimplementa aritmética). Es genérico: no conoce "tiempo", solo dibuja
// lo que el catálogo indica.
class HudOverlayPainter {
public:
	HudOverlayPainter(SdlContext& context, TextRenderer& text,
			  const world::HudOverlayCatalog& catalog);

	// `intensity` (0.0 a 1.0) escala el alpha de cada overlay: 1.0 los
	// dibuja a su opacidad plena (el HUD fijo) y 0.0 no dibuja nada
	// (la luz nocturna de día). Quién decide la intensidad es el dueño
	// del concepto (game::DayPhase), no este painter.
	void draw(double intensity = 1.0);

	// Escribe las etiquetas de texto del catálogo. `values` son los
	// valores CRUDOS que manda la fuente de estado, por clave; el
	// catálogo los interpreta (textFor) y TextRenderer los dibuja. Acá
	// no se decide qué dice cada etiqueta.
	void drawLabels(const std::map<std::string, int>& values);

private:
	// Rellena la ventana con un velo de color según su mezcla.
	void fillVeil(const world::HudOverlay& overlay, double intensity);
	void fillAlpha(const world::HudOverlay& overlay, double intensity);
	void fillMultiply(const world::HudOverlay& overlay, double intensity);

	// Cómo se rellena cada BlendMode. Tabla miembro (§9: mapa sobre
	// switch), inicializada en el constructor.
	using VeilFill = void (HudOverlayPainter::*)(const world::HudOverlay&,
						     double);

	SdlContext& context_;
	TextRenderer& text_;
	world::HudOverlayCatalog catalog_;
	world::AnchorLayout anchors_;
	SheetCache sheets_;
	std::map<world::BlendMode, VeilFill> veilFills_;
};

}
