#include "GameLoop.h"

#include "sdl/InputManager.h"

namespace sdl {

GameLoop::GameLoop(sdl::InputManager& input, std::function<void()> tick,
		   int targetFps)
	: input_(input)
	, tick_(std::move(tick))
	, frameMs_(targetFps > 0 ? 1000 / targetFps : 16)
	, running_(false) {}

void GameLoop::run() {

	running_ = true;
	while (running_) {
		const Uint32 frameStart = SDL_GetTicks();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running_ = false;
			} else {
				input_.handleEvent(event);
			}
		}
		if (!running_) {
			break;
		}

		tick_();

		// Compensar hasta el próximo frame para mantener la tasa.
		const Uint32 elapsed = SDL_GetTicks() - frameStart;
		if (elapsed < static_cast<Uint32>(frameMs_)) {
			SDL_Delay(frameMs_ - elapsed);
		}
	}
}

void GameLoop::stop() { running_ = false; }

}
