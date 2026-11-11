#pragma once

#include <SDL2pp/SDL2pp.hh>

#include <string>

#include "sdl/context/SdlConstants.h"

namespace core {
class WindowConfig;
}

namespace sdl {

class SdlContext {
public:
	explicit SdlContext(const core::WindowConfig& config);
	~SdlContext();

	SdlContext(const SdlContext&) = delete;
	SdlContext& operator=(const SdlContext&) = delete;

	void clear(const SDL2pp::Color& color);
	void fillRect(const SDL2pp::Rect& rect, const SDL2pp::Color& color);

	// Rellena multiplicando lo ya dibujado por el color (SDL_BLENDMODE_MOD):
	// el negro sigue negro, el resto se tiñe. Restaura la mezcla normal.
	void fillRectMultiply(const SDL2pp::Rect& rect,
			      const SDL2pp::Color& color);

	SDL2pp::Texture createTexture(const std::string& path);

	// Dibuja una superficie ya renderizada (p. ej. texto de una
	// fuente): la convierte en textura y la pega en dest.
	void drawSurface(SDL2pp::Surface& surface, const SDL2pp::Rect& dest);

	void drawTexture(SDL2pp::Texture& texture, const SDL2pp::Rect& src,
			 const SDL2pp::Rect& dest, Flip flip = Flip::None,
			 double angle = 0.0,
			 SDL2pp::Optional<SDL2pp::Point> center =
				 SDL2pp::NullOpt);

	void present();

	int width() const;
	int height() const;

private:
	SDL2pp::SDL sdl_;
	SDL2pp::SDLTTF sdlTtf_;
	SDL2pp::Window window_;
	SDL2pp::Renderer renderer_;
};

}
