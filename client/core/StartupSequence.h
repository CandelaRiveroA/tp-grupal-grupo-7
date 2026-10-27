#pragma once

namespace core {

enum class GamePhase { Intro, FadeIn, Playing };


GamePhase phaseForIteration(int it);

int fadeOutAlphaFor(int it);

}
