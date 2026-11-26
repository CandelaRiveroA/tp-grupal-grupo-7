#include "game/SceneTransition.h"

namespace game {

SceneTransition::SceneTransition(const int fadeTicks)
	: fadeTicks_(fadeTicks > 0 ? fadeTicks : 1) {}

void SceneTransition::changeTo(const std::string& scene,
			       const bool holdAtBlack) {
	if (phase_ != Phase::Idle) {
		if (!holdAtBlack && (phase_ == Phase::FadingOut || phase_ == Phase::Holding)) {
			// El mapa del servidor puede llegar después de DayEnd.
			// Todavía en negro: actualizar el destino antes de despertar.
			scene_ = scene;
		}
		// El fin de día puede llegar durante un cambio de mapa: no
		// perder la espera del despertar aunque ya haya un fade activo.
		if (holdAtBlack) {
			holdAtBlack_ = true;
			if (phase_ == Phase::FadingIn) {
				phase_ = Phase::FadingOut;
				startIteration_ = -1;
			}
		}
		// Una transición a la vez (POLA): la que está en curso sigue
		// hasta el final. Única excepción: el pedido de quedarse en
		// negro (fin del día), que se suma a la que corre.
		if (holdAtBlack) {
			upgradeToHold(scene);
		}
		return;
	}
	scene_ = scene;
	holdAtBlack_ = holdAtBlack;
	phase_ = Phase::FadingOut;
	startIteration_ = -1;
	resumeElapsed_ = 0;
	swapPending_ = false;
}

void SceneTransition::upgradeToHold(const std::string& scene) {
	if (phase_ == Phase::Holding) {
		return;
	}
	scene_ = scene;
	holdAtBlack_ = true;
	if (phase_ == Phase::FadingIn) {
		// Vuelve a oscurecer desde el negro que ya tenía.
		phase_ = Phase::FadingOut;
		startIteration_ = -1;
		resumeElapsed_ = alpha_ * fadeTicks_ / 255;
		swapPending_ = false;
	}
}

void SceneTransition::tick(const int iteration) {
	if (phase_ == Phase::Idle) {
		alpha_ = 0;
		return;
	}
	if (phase_ == Phase::Holding) {
		// Negro sostenido hasta release().
		alpha_ = 255;
		return;
	}
	if (startIteration_ < 0) {
		startIteration_ = iteration - resumeElapsed_;
		resumeElapsed_ = 0;
	}
	const int elapsed = iteration - startIteration_;

	if (phase_ == Phase::FadingOut) {
		if (elapsed < fadeTicks_) {
			alpha_ = 255 * elapsed / fadeTicks_;
			return;
		}
		alpha_ = 255;
		if (holdAtBlack_) {
			// Se queda en negro; el cambio de datos espera al OK.
			phase_ = Phase::Holding;
			return;
		}
		// Negro total: el instante del cambio de datos.
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

bool SceneTransition::holding() const { return phase_ == Phase::Holding; }

void SceneTransition::release() {
	if (phase_ != Phase::Holding) {
		return;
	}
	// Se suelta el negro: ahora sí cambian los datos y vuelve el fade.
	phase_ = Phase::FadingIn;
	startIteration_ = -1;
	swapPending_ = true;
}

int SceneTransition::alpha() const { return alpha_; }

bool SceneTransition::swapDue() const { return swapPending_; }

void SceneTransition::consumeSwap() { swapPending_ = false; }

const std::string& SceneTransition::scene() const { return scene_; }

}  // namespace game
