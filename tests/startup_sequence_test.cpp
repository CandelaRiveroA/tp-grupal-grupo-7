#include "gtest/gtest.h"

#include "core/Constants.h"
#include "core/StartupSequence.h"

namespace {

using core::fadeOutAlphaFor;
using core::GamePhase;
using core::phaseForIteration;
using core::kFadeTicks;
using core::kIntroTicks;

TEST(StartupSequenceTest, FirstIterationsAreTheIntro) {
	EXPECT_EQ(phaseForIteration(0), GamePhase::Intro);
	EXPECT_EQ(phaseForIteration(kIntroTicks - 1), GamePhase::Intro);
}

TEST(StartupSequenceTest, FadeStartsRightAfterTheIntro) {
	EXPECT_EQ(phaseForIteration(kIntroTicks), GamePhase::FadeIn);
	EXPECT_EQ(phaseForIteration(kIntroTicks + kFadeTicks - 1),
		  GamePhase::FadeIn);
}

TEST(StartupSequenceTest, PlayingStartsWhenTheSequenceEnds) {
	EXPECT_EQ(phaseForIteration(kIntroTicks + kFadeTicks),
		  GamePhase::Playing);
	EXPECT_EQ(phaseForIteration(kIntroTicks + kFadeTicks + 500),
		  GamePhase::Playing);
}

TEST(StartupSequenceTest, FadeStartsFullyOpaque) {
	EXPECT_EQ(fadeOutAlphaFor(kIntroTicks), 255);
}

TEST(StartupSequenceTest, FadeEndsNearlyTransparent) {
	const int alpha = fadeOutAlphaFor(kIntroTicks + kFadeTicks - 1);
	EXPECT_GT(alpha, 0);
	EXPECT_LT(alpha, 255);
}

TEST(StartupSequenceTest, FadeAlphaGoesDownEveryIteration) {
	for (int step = 1; step < kFadeTicks; ++step) {
		const int previous = fadeOutAlphaFor(kIntroTicks + step - 1);
		const int current = fadeOutAlphaFor(kIntroTicks + step);
		EXPECT_LE(current, previous) << "en el paso " << step;
	}
}

TEST(StartupSequenceTest, AlphaIsClampedOutsideTheFade) {
	EXPECT_EQ(fadeOutAlphaFor(0), 255);
	EXPECT_EQ(fadeOutAlphaFor(kIntroTicks + kFadeTicks + 100), 0);
}

TEST(StartupSequenceTest, SkippedIterationsLandOnTheirOwnPhase) {
	const int intoFade = kIntroTicks + 7;
	EXPECT_EQ(phaseForIteration(intoFade), GamePhase::FadeIn);
	EXPECT_LT(fadeOutAlphaFor(intoFade), fadeOutAlphaFor(kIntroTicks));

	const int pastEverything = kIntroTicks + kFadeTicks + 40;
	EXPECT_EQ(phaseForIteration(pastEverything), GamePhase::Playing);
}

}
