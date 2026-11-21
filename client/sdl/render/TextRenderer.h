#pragma once

#include <SDL2pp/SDL2pp.hh>

#include <map>
#include <string>
#include <utility>

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

	// Con tamaño propio y, opcionalmente, en negrita (el estilo bold de
	// SDL_ttf sobre la misma fuente: no hay un archivo bold aparte).
	void draw(const std::string& text, const world::PixelBox& box,
		  const world::Color& color, int size, bool bold);

	// Texto alineado a la izquierda: respeta los saltos de línea y
	// ajusta las palabras al ancho de la caja.
	void drawLeftWrapped(const std::string& text, const world::PixelBox& box,
			     const world::Color& color, int size, int lineGap);

private:
	SDL2pp::Font& fontFor(int size, bool bold);

	void drawLine(const std::string& line, int x, int y, SDL2pp::Font& font,
		      const SDL2pp::Color& ink);

	SdlContext& context_;
	world::FontCatalog font_;
	// Una fuente abierta por (tamaño, negrita).
	std::map<std::pair<int, bool>, SDL2pp::Font> fonts_;
};

}  // namespace sdl
