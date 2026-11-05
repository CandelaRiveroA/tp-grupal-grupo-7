#include "game/SceneTransition.h"

namespace game {

SceneTransition::SceneTransition(const int fadeTicks)
	: fadeTicks_(fadeTicks > 0 ? fadeTicks : 1) {}

void SceneTransition::changeTo(const std::string& scene) {
	if (phase_ != Phase::Idle) {
		// Una transición a la vez (POLA): la que está en curso sigue
		// hasta el final.
		return;
	}
	scene_ = scene;
	phase_ = Phase::FadingOut;
	startIteration_ = -1;
	swapPending_ = false;
}

void SceneTransition::tick(const int iteration) {
	if (phase_ == Phase::Idle) {
		alpha_ = 0;
		return;
	}
	if (startIteration_ < 0) {
		startIteration_ = iteration;
	}
	const int elapsed = iteration - startIteration_;

	if (phase_ == Phase::FadingOut) {
		if (elapsed < fadeTicks_) {
			alpha_ = 255 * elapsed / fadeTicks_;
			return;
		}
		// Negro total: el instante del cambio de datos.
		alpha_ = 255;
		phase_ = Phase::FadingIn;
		startIteration_ = iteration;
		swapPending_ = true;
		return;
	}

	// FadingIn: el negro baja hasta desaparecer.
	if (elapsed < fadeTicks_) {
		alpha_ = 255 * (fadeTicks_ - elapsed) / fadeTicks_;
		return;
	}
	alpha_ = 0;
	phase_ = Phase::Idle;
	startIteration_ = -1;
}

bool SceneTransition::active() const { return phase_ != Phase::Idle; }

int SceneTransition::alpha() const { return alpha_; }

bool SceneTransition::swapDue() const { return swapPending_; }

void SceneTransition::consumeSwap() { swapPending_ = false; }

const std::string& SceneTransition::scene() const { return scene_; }

}  // namespace game
