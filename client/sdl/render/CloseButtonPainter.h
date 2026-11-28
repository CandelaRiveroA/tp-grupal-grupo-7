#pragma once

#include "sdl/render/SheetCache.h"
#include "world/NpcVisuals.h"

namespace sdl {
class SdlContext;
}

namespace sdl {

// Pinta el botón de cerrar de un popup en la esquina de su panel. La caja
// la calcula world::CloseButton; esto solo dibuja la celda en esa caja.
class CloseButtonPainter {
public:
	explicit CloseButtonPainter(SdlContext& context);

	void draw(const world::CloseButton& button, const world::PixelBox& panel);

private:
	SheetCache sheets_;
};

}
