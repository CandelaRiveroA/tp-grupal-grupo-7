#include "sdl/render/TextRenderer.h"

#include "core/Constants.h"
#include "sdl/context/SdlContext.h"

namespace sdl {

TextRenderer::TextRenderer(SdlContext& context, const world::FontCatalog& font)
	: context_(context), font_(font) {}

void TextRenderer::draw(const std::string& text, const world::PixelBox& box,
			const world::Color& color) {
	draw(text, box, color, font_.size);
}

void TextRenderer::draw(const std::string& text, const world::PixelBox& box,
			const world::Color& color, const int size) {
	if (text.empty()) {
		return;
	}
	SDL2pp::Font& font = fontFor(size);
	const SDL2pp::Color ink(static_cast<Uint8>(color.r),
				static_cast<Uint8>(color.g),
				static_cast<Uint8>(color.b),
				static_cast<Uint8>(color.a));
	SDL2pp::Surface surface = font.RenderUTF8_Blended(text, ink);
	const SDL2pp::Point measured = font.GetSizeUTF8(text);
	const int x = box.x + (box.width - measured.x) / 2;
	const int y = box.y + (box.height - measured.y) / 2;
	context_.drawSurface(surface, SDL2pp::Rect(x, y, measured.x, measured.y));
}

SDL2pp::Font& TextRenderer::fontFor(const int size) {
	const std::map<int, SDL2pp::Font>::const_iterator found =
		fonts_.find(size);
	if (found != fonts_.end()) {
		return fonts_.at(size);
	}
	// Una fuente abierta por tamaño, cacheada (SDL2pp::Font no es
	// copiable: se emplacea en el mapa y se consulta de ahí).
	return fonts_
		.emplace(size,
			 SDL2pp::Font(core::assetPath(font_.file), size))
		.first->second;
}

}  // namespace sdl
