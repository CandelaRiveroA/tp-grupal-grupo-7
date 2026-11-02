#pragma once

#include "game/LocalWorldView.h"
#include "sdl/render/SheetCache.h"
#include "world/DialogCatalog.h"

namespace sdl {
class SdlContext;
class Viewport;
}

namespace sdl {

// Pinta el diálogo de una entidad: el frame actual de su animación de
// aparición y, cuando termina, su contenido centrado (lo que "informa").
// Solo dibuja: QUÉ indicación mostrar lo decide el estado de juego y
// CÓMO se ve, el catálogo. Los rectángulos salen de world::DialogLayout.
class DialogPainter {
public:
	explicit DialogPainter(SdlContext& context);

	// Dibuja el diálogo sobre la caja de mundo de la entidad. Los
	// rectángulos pasan por el viewport como todo dibujo de mundo (§11).
	void draw(const world::DialogCatalog& catalog,
		  world::Indication indication, const world::PixelRect& entityBox,
		  int animationTick, const Viewport& viewport);

private:
	SheetCache sheets_;
};

}  // namespace sdl
