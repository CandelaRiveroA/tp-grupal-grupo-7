#include "core/StartupSequence.h"

#include <algorithm>

#include "core/Constants.h"

namespace core {

GamePhase phaseForIteration(int it) {
	if (it < kIntroTicks) {
		return GamePhase::Intro;
	}
	if (it < kIntroTicks + kFadeTicks) {
		return GamePhase::FadeIn;
	}
	return GamePhase::Playing;
}

int fadeOutAlphaFor(int it) {
	const int elapsed = it - kIntroTicks;
	const int remaining = kFadeTicks - elapsed;
	return std::clamp(kFadeOpaqueAlpha * remaining / kFadeTicks, 0,
			  kFadeOpaqueAlpha);
}

}
