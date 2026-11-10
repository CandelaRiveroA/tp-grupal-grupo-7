#include <chrono>

#include "gtest/gtest.h"

#include "../common/common_frame_clock.h"

namespace {

using std::chrono::milliseconds;
using FrameClock = sdl::FrameClock;

constexpr milliseconds kRate = milliseconds(16);

FrameClock::TimePoint at(milliseconds offset) {
	return FrameClock::TimePoint{} + offset;
}

TEST(FrameClockTest, RestartStartsAtIterationZero) {
	FrameClock clock(kRate);
	clock.restart(at(milliseconds(1000)));
	EXPECT_EQ(clock.iteration(), 0);
}

TEST(FrameClockTest, OnTimeTickSleepsTheRemainderOfTheRate) {
	FrameClock clock(kRate);
	clock.restart(at(milliseconds(0)));
	EXPECT_EQ(clock.advance(at(milliseconds(10))), milliseconds(6));
	EXPECT_EQ(clock.iteration(), 1);
}

TEST(FrameClockTest, TickExactlyOnTheDeadlineSleepsZero) {
	FrameClock clock(kRate);
	clock.restart(at(milliseconds(0)));
	EXPECT_EQ(clock.advance(at(milliseconds(16))), milliseconds(0));
	EXPECT_EQ(clock.iteration(), 1);
}

TEST(FrameClockTest, SleepOvershootDoesNotAccumulateDrift) {
	FrameClock clock(kRate);
	clock.restart(at(milliseconds(0)));
	EXPECT_EQ(clock.advance(at(milliseconds(10))),
		  milliseconds(6));
	EXPECT_EQ(clock.advance(at(milliseconds(18))),
		  milliseconds(14));
	EXPECT_EQ(clock.iteration(), 2);
}

TEST(FrameClockTest, SlightlyLateKeepsWorkingWithoutSleeping) {
	FrameClock clock(kRate);
	clock.restart(at(milliseconds(0)));
	EXPECT_EQ(clock.advance(at(milliseconds(20))), milliseconds(0));
	EXPECT_EQ(clock.iteration(), 1);
}

TEST(FrameClockTest, FullyConsumedIterationsAreSkipped) {
	FrameClock clock(kRate);
	clock.restart(at(milliseconds(0)));
	EXPECT_EQ(clock.advance(at(milliseconds(40))), milliseconds(0));
	EXPECT_EQ(clock.iteration(), 2);
}

TEST(FrameClockTest, LosingIterationsSkipsExactlyTheFullyConsumedOnes) {
	FrameClock clock(kRate);
	clock.restart(at(milliseconds(0)));
	EXPECT_EQ(clock.advance(at(milliseconds(48))), milliseconds(0));
	EXPECT_EQ(clock.iteration(), 3);
}

TEST(FrameClockTest, ScheduleRecoversAfterBeingBehind) {
	FrameClock clock(kRate);
	clock.restart(at(milliseconds(0)));
	clock.advance(at(milliseconds(40)));
	EXPECT_EQ(clock.advance(at(milliseconds(40))), milliseconds(8));
	EXPECT_EQ(clock.iteration(), 3);
}

TEST(FrameClockTest, RestartResetsTheWholeSchedule) {
	FrameClock clock(kRate);
	clock.restart(at(milliseconds(0)));
	clock.advance(at(milliseconds(40)));
	clock.restart(at(milliseconds(500)));
	EXPECT_EQ(clock.iteration(), 0);
	EXPECT_EQ(clock.advance(at(milliseconds(510))), milliseconds(6));
}

TEST(FrameClockTest, BuildsItsRateFromFps) {
	FrameClock clock(60);
	clock.restart(at(milliseconds(0)));
	EXPECT_EQ(clock.advance(at(milliseconds(0))),
		  std::chrono::nanoseconds(1'000'000'000 / 60));
}

}  // namespace
