#include <vector>

#include "game/ResourceAudioCue.h"
#include "gtest/gtest.h"

TEST(ResourceAudioCueTest, NoImpactDoesNotFire) {
    game::ResourceAudioCue cue;
    EXPECT_TRUE(cue.update({}).empty());
}

TEST(ResourceAudioCueTest, ConsecutiveHitsDoNotNeedAVisualChange) {
    game::ResourceAudioCue cue;
    for (int i = 0; i < 3; ++i) {
        const auto events = cue.update({{1, 1, false}});
        ASSERT_EQ(events.size(), 1u);
        EXPECT_EQ(events[0], game::ResourceAudioEvent::Hit);
    }
    EXPECT_TRUE(cue.update({}).empty());
}

TEST(ResourceAudioCueTest, FinalImpactOnlyFiresBreakSound) {
    game::ResourceAudioCue cue;
    const auto events = cue.update({{1, 1, true}});
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0], game::ResourceAudioEvent::Broke);
}

TEST(ResourceAudioCueTest, MultipleImpactsInOneTickPreserveOrderAndCount) {
    game::ResourceAudioCue cue;
    const auto events = cue.update({{1, 1, false}, {2, 2, true}, {1, 1, false}});
    const std::vector<game::ResourceAudioEvent> expected = {game::ResourceAudioEvent::Hit,
                                                            game::ResourceAudioEvent::Broke,
                                                            game::ResourceAudioEvent::Hit};
    EXPECT_EQ(events, expected);
}
