#include "sdl/loop/GameLoop.h"

#include <SDL2/SDL.h>

#include <chrono>
#include <thread>
#include <utility>

#include "sdl/input/InputManager.h"

namespace sdl {

GameLoop::GameLoop(sdl::InputManager& input, std::function<void(int)> tick,
		   int targetFps)
	: input_(input)
	, tick_(std::move(tick))
	, clock_(targetFps > 0 ? targetFps : core::kTargetFps)
	, running_(false) {}

void GameLoop::run() {
	running_ = true;
	clock_.restart(FrameClock::Clock::now());

	while (running_) {
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

		tick_(clock_.iteration());

		const FrameClock::Duration rest =
			clock_.advance(FrameClock::Clock::now());
		std::this_thread::sleep_for(rest);
	}
}

}
