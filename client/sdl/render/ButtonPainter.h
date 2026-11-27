#pragma once

#include <map>
#include <string>

#include "sdl/render/SpriteSheet.h"
#include "world/PixelBox.h"
#include "world/PromptLayout.h"

namespace sdl {
class SdlContext;
}

namespace sdl {

// Dibuja los botones 9-slice de ui/text.toml ([prompt.<botón>]: cancel,
// ok, ...) en la caja que le pasen. Los usan el popup del sueño y el de
// configuración; cada uno decide dónde va el botón, esto solo lo pinta.
class ButtonPainter {
public:
	ButtonPainter(SdlContext& context, const world::PromptLayout& layout);

	// El botón `name` con sus 9 piezas estiradas a la caja.
	void draw(const std::string& name, const world::PixelBox& box);

private:
	const world::PromptLayout& layout_;
	std::map<std::string, SpriteSheet> sheets_;
};

}  // namespace sdl
