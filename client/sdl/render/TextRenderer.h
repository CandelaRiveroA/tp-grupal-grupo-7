#pragma once

#include <SDL2pp/SDL2pp.hh>

#include <map>
#include <string>

#include "world/Color.h"
#include "world/FontCatalog.h"
#include "world/InventoryLayout.h"

namespace sdl {
class SdlContext;
}

namespace sdl {

// Único dueño de la fuente de texto (SDL2pp::Font): la herramienta
// genérica "poner texto" del cliente. Cualquier HUD, popup o aviso que
// necesite letras dibuja por acá (texto centrado en una caja, con un
// color); nadie más toca SDL_ttf. Las fuentes por tamaño se cachean.
class TextRenderer {
public:
	TextRenderer(SdlContext& context, const world::FontCatalog& font);

	// Dibuja el texto centrado en la caja, con el tamaño por defecto de
	// la fuente.
	void draw(const std::string& text, const world::PixelBox& box,
		  const world::Color& color);

	// Lo mismo con un tamaño de fuente propio (px).
	void draw(const std::string& text, const world::PixelBox& box,
		  const world::Color& color, int size);

	
	void drawLeft(const std::string& text, const world::PixelBox& box,
		      const world::Color& color, int size);

	// Ancho en px del texto con esa fuente (wrap y cursor).
	int textWidth(const std::string& text, int size);

private:
	SDL2pp::Font& fontFor(int size);

	SdlContext& context_;
	world::FontCatalog font_;
	std::map<int, SDL2pp::Font> fonts_;
};

}  // namespace sdl
