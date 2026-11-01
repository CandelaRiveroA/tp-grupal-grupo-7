#include "game/FloorItems.h"

#include <optional>

#include "gtest/gtest.h"

#include "game/GameConstants.h"

namespace {

using game::FloorItems;
using game::InteractionTarget;
using world::ItemKind;

TEST(FloorItemsTest, DropsAndLists) {
    FloorItems floor;
    EXPECT_TRUE(floor.tryDrop(ItemKind::Egg, 3, 4));
    ASSERT_EQ(floor.items().size(), 1u);
    EXPECT_EQ(floor.items()[0].kind, ItemKind::Egg);
    EXPECT_EQ(floor.items()[0].cellX, 3);
    EXPECT_EQ(floor.items()[0].cellY, 4);
}

TEST(FloorItemsTest, OneItemPerCell) {
    FloorItems floor;
    EXPECT_TRUE(floor.tryDrop(ItemKind::Egg, 3, 4));
    EXPECT_FALSE(floor.tryDrop(ItemKind::Milk, 3, 4));
    EXPECT_EQ(floor.items().size(), 1u);
}

TEST(FloorItemsTest, NeverExceedsTheCap) {
    FloorItems floor;
    for (int i = 0; i < game::kMaxGroundItems; ++i) {
        EXPECT_TRUE(floor.tryDrop(ItemKind::Egg, i, 0));
    }
    EXPECT_FALSE(floor.tryDrop(ItemKind::Egg, 9, 9));
    EXPECT_EQ(floor.items().size(),
              static_cast<std::size_t>(game::kMaxGroundItems));
}

TEST(FloorItemsTest, FindsTargetNearThePlayer) {
    FloorItems floor;
    ASSERT_TRUE(floor.tryDrop(ItemKind::Egg, 5, 5));

    const std::optional<InteractionTarget> near =
            floor.findTargetNear(5, 6);  // adyacente
    ASSERT_TRUE(near.has_value());
    EXPECT_EQ(near->action, world::ActionKind::Pickup);
    EXPECT_EQ(near->reward, "egg");
    ASSERT_TRUE(near->held.has_value());
    EXPECT_EQ(*near->held, ItemKind::Egg);
    EXPECT_EQ(near->cellX, 5);
    EXPECT_EQ(near->cellY, 5);

    EXPECT_FALSE(floor.findTargetNear(9, 9).has_value());
}

TEST(FloorItemsTest, StartedTargetLeavesTheFloor) {
    FloorItems floor;
    ASSERT_TRUE(floor.tryDrop(ItemKind::Egg, 5, 5));
    const InteractionTarget target = *floor.findTargetNear(5, 5);

    floor.onInteractionStarted(target);
    EXPECT_TRUE(floor.items().empty());
    // Terminar después no rompe nada (ya salió al empezar).
    floor.onInteractionFinished(target);
    EXPECT_TRUE(floor.items().empty());
}

}  // namespace
