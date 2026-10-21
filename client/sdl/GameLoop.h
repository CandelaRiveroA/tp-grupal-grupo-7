#pragma once

#include <SDL2/SDL.h>

#include <functional>

namespace sdl {
class InputManager;
}

namespace sdl {

// Constant rate loop: corre el juego a tasa (casi) constante sin depender
// de ningún event loop externo. Cada ciclo: drena eventos SDL (input y
// quit), ejecuta el tick y compensa el tiempo restante del frame.
class GameLoop {
public:
	GameLoop(sdl::InputManager& input, std::function<void()> tick,
		 int targetFps = 60);

	// Bloquea hasta SDL_QUIT o stop().
	void run();
	void stop();

private:
	sdl::InputManager& input_;
	std::function<void()> tick_;
	int frameMs_;
	bool running_;
};

}
