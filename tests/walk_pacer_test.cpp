#include <optional>

#include "game/WalkPacer.h"
#include "gtest/gtest.h"
#include "world/CharacterCatalog.h"
#include "world/CharacterCatalogLoader.h"
#include "world/CreatureCatalog.h"

namespace {

using game::WalkIntent;
using game::WalkPacer;
using world::Direction;

const char* const kPlayerToml = R"(
[characters.player]
sheet   = "walking.png"
grid    = [16, 32]
drawn   = [24, 48]
cadence = { step = 24, frame = 12 }
  [characters.player.front]
  mirrored = false
    [characters.player.front.idle]
    loops  = false
    frames = [ [[0, 0], [6, 0]] ]
    [characters.player.front.walk]
    loops  = true
    frames = [ [[1, 0], [7, 0]], [[2, 0], [8, 0]] ]
  [characters.player.back]
  mirrored = false
    [characters.player.back.idle]
    loops  = false
    frames = [ [[0, 2], [6, 2]] ]
    [characters.player.back.walk]
    loops  = true
    frames = [ [[1, 2], [7, 2]], [[2, 2], [8, 2]] ]
  [characters.player.left]
  mirrored = true
    [characters.player.left.idle]
    loops  = false
    frames = [ [[0, 1], [6, 1]] ]
    [characters.player.left.walk]
    loops  = true
    frames = [ [[1, 1], [7, 1]], [[2, 1], [8, 1]] ]
  [characters.player.right]
  mirrored = false
    [characters.player.right.idle]
    loops  = false
    frames = [ [[0, 1], [6, 1]] ]
    [characters.player.right.walk]
    loops  = true
    frames = [ [[1, 1], [7, 1]], [[2, 1], [8, 1]] ]
)";

world::MoveCadence playerCadence() {
    static const world::CharacterCatalog catalog =
            world::CharacterCatalogLoader().loadFromString(kPlayerToml);
    return catalog.dataFor(world::CreatureKind::Player).cadence;
}

std::optional<Direction> holding(const Direction direction) { return direction; }


TEST(WalkPacerTest, FirstContactStepsImmediately) {
    WalkPacer pacer(playerCadence());
    EXPECT_EQ(pacer.intentFor(0, holding(Direction::Right)), WalkIntent::Step);
    EXPECT_EQ(pacer.lastDirection(), Direction::Right);
}

TEST(WalkPacerTest, HeldKeyStepsOnceEveryCadenceTicks) {
    constexpr int kStepTicks = 24;
    ASSERT_EQ(playerCadence().stepTicks, kStepTicks);

    WalkPacer pacer(playerCadence());
    EXPECT_EQ(pacer.intentFor(0, holding(Direction::Right)), WalkIntent::Step);
    for (int it = 1; it < kStepTicks; ++it) {
        EXPECT_EQ(pacer.intentFor(it, holding(Direction::Right)), WalkIntent::None)
                << "en la iteración " << it;
    }
    EXPECT_EQ(pacer.intentFor(kStepTicks, holding(Direction::Right)), WalkIntent::Step);
    EXPECT_EQ(pacer.intentFor(kStepTicks + 1, holding(Direction::Right)), WalkIntent::None);
}

TEST(WalkPacerTest, ReleasingEmitsASingleStop) {
    WalkPacer pacer(playerCadence());
    EXPECT_EQ(pacer.intentFor(0, holding(Direction::Right)), WalkIntent::Step);
    EXPECT_EQ(pacer.intentFor(1, std::nullopt), WalkIntent::Stop);
    EXPECT_EQ(pacer.intentFor(2, std::nullopt), WalkIntent::None);
    EXPECT_EQ(pacer.intentFor(50, std::nullopt), WalkIntent::None);
}

TEST(WalkPacerTest, NeverPressedNeverStops) {
    WalkPacer pacer(playerCadence());
    EXPECT_EQ(pacer.intentFor(0, std::nullopt), WalkIntent::None);
    EXPECT_EQ(pacer.intentFor(1000, std::nullopt), WalkIntent::None);
}

TEST(WalkPacerTest, PressingAgainAfterReleasingStepsImmediately) {
    WalkPacer pacer(playerCadence());
    pacer.intentFor(0, holding(Direction::Right));
    pacer.intentFor(1, std::nullopt);
    EXPECT_EQ(pacer.intentFor(2, holding(Direction::Front)), WalkIntent::Step);
    EXPECT_EQ(pacer.lastDirection(), Direction::Front);
}

TEST(WalkPacerTest, LostIterationsDoNotBurstSteps) {
    WalkPacer pacer(playerCadence());
    EXPECT_EQ(pacer.intentFor(0, holding(Direction::Right)), WalkIntent::Step);
    EXPECT_EQ(pacer.intentFor(100, holding(Direction::Right)), WalkIntent::Step);
    EXPECT_EQ(pacer.intentFor(101, holding(Direction::Right)), WalkIntent::None);
    EXPECT_EQ(pacer.intentFor(100 + playerCadence().stepTicks, holding(Direction::Right)),
              WalkIntent::Step);
}

TEST(WalkPacerTest, CadenceIsReceivedAndNotHardcoded) {
    const world::MoveCadence quick{10, 5};
    WalkPacer pacer(quick);
    EXPECT_EQ(pacer.intentFor(0, holding(Direction::Left)), WalkIntent::Step);
    EXPECT_EQ(pacer.intentFor(9, holding(Direction::Left)), WalkIntent::None);
    EXPECT_EQ(pacer.intentFor(10, holding(Direction::Left)), WalkIntent::Step);
}

TEST(WalkPacerTest, FollowsTheLatestDirectionPressedWhileHeld) {
    WalkPacer pacer(playerCadence());
    EXPECT_EQ(pacer.intentFor(0, holding(Direction::Right)), WalkIntent::Step);
    EXPECT_EQ(pacer.intentFor(5, holding(Direction::Back)), WalkIntent::None);
    EXPECT_EQ(pacer.lastDirection(), Direction::Back);
    EXPECT_EQ(pacer.intentFor(playerCadence().stepTicks, holding(Direction::Back)),
              WalkIntent::Step);
    EXPECT_EQ(pacer.lastDirection(), Direction::Back);
}

}  // namespace
