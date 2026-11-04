#include <vector>

#include "gtest/gtest.h"

#include "world/CreatureCatalog.h"

namespace {

using world::ActionData;
using world::ActionKind;
using world::CreatureData;

ActionData actionWithFrames(int frames, int frameTicks) {
    ActionData data;
    data.frames = std::vector<world::Pose>(static_cast<std::size_t>(frames));
    data.frameTicks = frameTicks;
    return data;
}

TEST(CreatureDataTest, UseToolDurationIsFramesTimesFrameTicks) {
    CreatureData creature{};
    creature.directions[0].actions[ActionKind::UseTool] = actionWithFrames(3, 12);
    EXPECT_EQ(creature.toolUseDurationTicks(), 36);
}

TEST(CreatureDataTest, FindsUseToolInAnyDirection) {
    CreatureData creature{};
    creature.directions[2].actions[ActionKind::UseTool] = actionWithFrames(2, 10);
    EXPECT_EQ(creature.toolUseDurationTicks(), 20);
}

TEST(CreatureDataTest, NonPositiveDurationFallsBackToOne) {
    CreatureData creature{};

    creature.directions[0].actions[ActionKind::UseTool] = actionWithFrames(0, 12);
    EXPECT_EQ(creature.toolUseDurationTicks(), 1);

    creature.directions[0].actions[ActionKind::UseTool] = actionWithFrames(3, 0);
    EXPECT_EQ(creature.toolUseDurationTicks(), 1);
}

TEST(CreatureDataTest, MissingUseToolFallsBackToOne) {
    CreatureData creature{};
    creature.directions[0].actions[ActionKind::Walk] = actionWithFrames(4, 8);
    EXPECT_EQ(creature.toolUseDurationTicks(), 1);
}

TEST(CreatureDataTest, ActionCycleTicksUsesFramesTimesFrameTicks) {
    CreatureData creature{};
    creature.directions[0].actions[ActionKind::Milk] = actionWithFrames(2, 6);
    // Un ciclo de la animación (2 poses x 6 ticks): la ventana del
    // ordeñe se mide con esta misma cuenta.
    EXPECT_EQ(creature.actionCycleTicks(ActionKind::Milk), 12);
    // La acción no existe: 0 (sin ventana).
    EXPECT_EQ(creature.actionCycleTicks(ActionKind::Walk), 0);
}

}  // namespace
