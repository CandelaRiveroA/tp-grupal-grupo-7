#pragma once

#include <vector>

#include "sdl/render/SpriteSheet.h"
#include "world/DigitCatalog.h"
#include "world/PixelBox.h"

namespace sdl {
class SdlContext;
}

namespace sdl {

// Dibuja números con la hoja de dígitos (digits.png). Solo pinta: qué
// dígitos y en qué cajas lo deciden quienes lo usan (la cantidad del
// inventario, el porcentaje del volumen), cada uno con su geometría.
class DigitPainter {
public:
	DigitPainter(SdlContext& context, const world::DigitCatalog& digits);

	// Cada dígito en su caja (digits[i] en boxes[i]); lo que sobre en
	// cualquiera de las dos listas se ignora.
	void draw(const std::vector<int>& digits,
		  const std::vector<world::PixelBox>& boxes);

private:
	world::DigitCatalog digits_;
	SpriteSheet sheet_;
};

}  // namespace sdl
