#include "gtest/gtest.h"

#include "game/ToolUse.h"

namespace {

using game::ToolUse;


TEST(ToolUseTest, InactiveBeforeAnyTrigger) {
	ToolUse toolUse(60);
	EXPECT_FALSE(toolUse.active(0));
	EXPECT_FALSE(toolUse.active(1000));
}

TEST(ToolUseTest, ActiveFromTheTriggerForTheWholeDuration) {
	ToolUse toolUse(60);
	toolUse.trigger(100);
	EXPECT_TRUE(toolUse.active(100));
	EXPECT_TRUE(toolUse.active(100 + 30));
	EXPECT_TRUE(toolUse.active(100 + 60 - 1));
}

TEST(ToolUseTest, InactiveOnceTheDurationElapses) {
	ToolUse toolUse(60);
	toolUse.trigger(100);
	EXPECT_FALSE(toolUse.active(100 + 60));
	EXPECT_FALSE(toolUse.active(100 + 600));
}

TEST(ToolUseTest, ReTriggerRestartsTheWindow) {
	ToolUse toolUse(60);
	toolUse.trigger(100);
	toolUse.trigger(200);
	EXPECT_TRUE(toolUse.active(200 + 60 - 1));
	EXPECT_FALSE(toolUse.active(200 + 60));
	EXPECT_FALSE(toolUse.active(160));
}

TEST(ToolUseTest, OneTickDurationIsASingleTickGolpe) {
	ToolUse toolUse(1);
	toolUse.trigger(50);
	EXPECT_TRUE(toolUse.active(50));
	EXPECT_FALSE(toolUse.active(51));
}

TEST(ToolUseTest, IterationsBeforeTheTriggerDoNotActivateIt) {
	ToolUse toolUse(60);
	toolUse.trigger(100);
	EXPECT_FALSE(toolUse.active(99));
	EXPECT_FALSE(toolUse.active(0));
}

TEST(ToolUseTest, ElapsedTicksMeasureFromTheTriggerNotFromTheClock) {
	ToolUse toolUse(60);
	EXPECT_EQ(toolUse.elapsedTicks(50000), 0);
	toolUse.trigger(100000);
	EXPECT_EQ(toolUse.elapsedTicks(100000), 0);
	EXPECT_EQ(toolUse.elapsedTicks(100000 + 12), 12);
	EXPECT_EQ(toolUse.elapsedTicks(100000 + 59), 59);
	EXPECT_EQ(toolUse.elapsedTicks(99999), 0);
}


world::ActionData oneShot(int frames, int frameTicks) {
	world::ActionData action;
	action.loops = false;
	action.frameTicks = frameTicks;
	action.frames.resize(static_cast<std::size_t>(frames));
	return action;
}

TEST(ToolUseTest, WindowLastsTheActionDurationFromItsOwnData) {
	ToolUse toolUse(oneShot(5, 8));
	toolUse.trigger(100);
	EXPECT_TRUE(toolUse.active(100));
	EXPECT_TRUE(toolUse.active(100 + 39));
	EXPECT_FALSE(toolUse.active(100 + 40));
}

TEST(ToolUseTest, WindowFollowsTheActionFrameRate) {
	ToolUse faster(oneShot(5, 6));
	faster.trigger(0);
	EXPECT_TRUE(faster.active(29));
	EXPECT_FALSE(faster.active(30));
}

TEST(ToolUseTest, InvalidActionDataStillYieldsATickWindow) {
	ToolUse toolUse(oneShot(5, 0));
	toolUse.trigger(0);
	EXPECT_TRUE(toolUse.active(0));
	EXPECT_FALSE(toolUse.active(1));
}

}
