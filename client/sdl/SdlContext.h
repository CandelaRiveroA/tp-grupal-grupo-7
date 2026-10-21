#pragma once

#include <SDL2pp/SDL2pp.hh>

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

	void readPixels(void* dst, int pitch);

private:
	SDL2pp::SDL sdl_;
	SDL2pp::Window window_;
	SDL2pp::Renderer renderer_;
	SDL2pp::Texture texture_;
};

}
