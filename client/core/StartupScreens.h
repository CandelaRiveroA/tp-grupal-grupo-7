#pragma once

#include <SDL2pp/SDL2pp.hh>

#include "core/StartupSequence.h"

namespace game {
class LocalWorldView;
class ViewMotion;
class ActionResolver;
}
namespace sdl {
class AudioPlayer;
class Renderer;
class SdlContext;
class Viewport;
}
namespace world {
class Scenario;
}

namespace core {

class StartupScreens {
public:
	StartupScreens(sdl::SdlContext& context, sdl::Renderer& renderer,
		       sdl::AudioPlayer& audio,
		       const world::Scenario& scenario,
		       const sdl::Viewport& viewport);

	bool draw(GamePhase phase, int it,
		  const game::LocalWorldView& state,
		  const game::ViewMotion& motion,
		  const game::ActionResolver& actionResolver) const;

private:
	void drawIntro(int it) const;
	void drawFadeIn(int it, const game::LocalWorldView& state,
			const game::ViewMotion& motion,
			const game::ActionResolver& actionResolver) const;

	sdl::SdlContext& context_;
	sdl::Renderer& renderer_;
	sdl::AudioPlayer& audio_;
	const world::Scenario& scenario_;
	const sdl::Viewport& viewport_;

	const SDL2pp::Color introBackground_;
};

}
