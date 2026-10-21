#include "SdlContext.h"

#include <SDL2/SDL.h>

#include "core/WindowConfig.h"

namespace sdl {

SdlContext::SdlContext(const core::WindowConfig& config)
	: sdl_(SDL_INIT_VIDEO)
	, window_("sdl-offscreen",
		  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		  config.width(), config.height(),
		  SDL_WINDOW_HIDDEN)
	, renderer_(window_, -1, SDL_RENDERER_ACCELERATED)
	, texture_(renderer_,
		   SDL_PIXELFORMAT_ARGB8888,
		   SDL_TEXTUREACCESS_TARGET,
		   config.width(), config.height()) {

	renderer_.SetTarget(texture_);
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

void SdlContext::readPixels(void* dst, int pitch) {

	renderer_.ReadPixels(SDL2pp::NullOpt,
			     SDL_PIXELFORMAT_ARGB8888,
			     dst, pitch);
}

}
