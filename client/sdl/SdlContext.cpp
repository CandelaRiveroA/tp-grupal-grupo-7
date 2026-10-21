#include "SdlContext.h"

#include <SDL2/SDL.h>

#include "core/WindowConfig.h"

namespace sdl {

SdlContext::SdlContext(const core::WindowConfig& config)
	: sdl_(SDL_INIT_VIDEO)
	, window_(config.title(),
		  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		  config.width(), config.height(),
		  SDL_WINDOW_SHOWN)
	, renderer_(window_, -1, SDL_RENDERER_ACCELERATED) {}

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

}
