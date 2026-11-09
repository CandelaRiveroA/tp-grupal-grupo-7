#include "core/StartupScreens.h"

#include "core/Constants.h"
#include "sdl/audio/AudioPlayer.h"
#include "sdl/context/SdlContext.h"
#include "sdl/render/Renderer.h"
#include "world/Scenario.h"

namespace core {

StartupScreens::StartupScreens(sdl::SdlContext& context,
			       sdl::Renderer& renderer,
			       sdl::AudioPlayer& audio,
			       const world::Scenario& scenario,
			       const sdl::Viewport& viewport)
	: context_(context)
	, renderer_(renderer)
	, audio_(audio)
	, scenario_(scenario)
	, viewport_(viewport)
	, introBackground_(0, 0, 0, 255) {}

bool StartupScreens::draw(GamePhase phase, int it,
			  const game::LocalWorldView& state,
			  const game::ViewMotion& motion,
			  const game::ActionResolver& actionResolver,
			  const world::TimeOfDay timeOfDay) const {
	switch (phase) {
	case GamePhase::Intro:
		drawIntro(it);
		return true;
	case GamePhase::FadeIn:
		drawFadeIn(it, state, motion, actionResolver, timeOfDay);
		return true;
	case GamePhase::Playing:
		break;
	}
	return false;
}

void StartupScreens::drawIntro(int it) const {
	if (it == kFirstIteration) {
		audio_.play(sdl::SoundId::RoosterCrow);
	}
	context_.clear(introBackground_);
	context_.present();
}

void StartupScreens::drawFadeIn(
	int it, const game::LocalWorldView& state,
	const game::ViewMotion& motion,
	const game::ActionResolver& actionResolver,
	const world::TimeOfDay timeOfDay) const {
	renderer_.drawWorld(scenario_.scene(), state, motion,
			    actionResolver, it, viewport_, timeOfDay);
	const int alpha = fadeOutAlphaFor(it);
	context_.fillRect(
		SDL2pp::Rect(0, 0, context_.width(), context_.height()),
		SDL2pp::Color(0, 0, 0, static_cast<Uint8>(alpha)));
	context_.present();
}

}
