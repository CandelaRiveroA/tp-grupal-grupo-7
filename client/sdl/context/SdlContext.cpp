#include "SdlContext.h"

#include <SDL2/SDL.h>

#include "core/WindowConfig.h"
#include "sdl/context/SdlConstants.h"

namespace sdl {

SdlContext::SdlContext(const core::WindowConfig& config)
	: sdl_(SDL_INIT_VIDEO)
	, window_(config.title(),
		  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		  config.width(), config.height(),
		  SDL_WINDOW_SHOWN)
	, renderer_(window_, -1, SDL_RENDERER_ACCELERATED) {

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, kNearestScaleQuality);

	// El puntero se dibuja con el asset del juego (CursorPainter): el
	// del sistema se oculta para no tener dos cursores.
	SDL_ShowCursor(SDL_DISABLE);

	renderer_.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
}

SdlContext::~SdlContext() = default;

void SdlContext::clear(const SDL2pp::Color& color) {
	renderer_.SetDrawColor(color);
	renderer_.Clear();
}

void SdlContext::fillRect(const SDL2pp::Rect& rect, const SDL2pp::Color& color) {

	renderer_.SetDrawColor(color);
	renderer_.FillRect(rect);
}

void SdlContext::present() { renderer_.Present(); }

int SdlContext::width() const { return window_.GetSize().x; }

int SdlContext::height() const { return window_.GetSize().y; }

SDL2pp::Texture SdlContext::createTexture(const std::string& path) {
	return SDL2pp::Texture(renderer_, path);
}

void SdlContext::drawTexture(SDL2pp::Texture& texture,
			     const SDL2pp::Rect& src,
			     const SDL2pp::Rect& dest, Flip flip,
			     double angle,
			     SDL2pp::Optional<SDL2pp::Point> center) {
	if (flip == Flip::None && angle == 0.0) {
		renderer_.Copy(texture, src, dest);
		return;
	}
	const SDL_RendererFlip sdlFlip = (flip == Flip::None)
		? SDL_FLIP_NONE
		: SDL_FLIP_HORIZONTAL;
	// center = NullOpt → SDL rota sobre el centro del dest; si viene un
	// punto, rota sobre ese punto (pivote del mango de la herramienta).
	renderer_.Copy(texture, src, dest, angle, center, sdlFlip);
}

}
