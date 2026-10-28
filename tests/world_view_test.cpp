#include "world_view.h"

#include <memory>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "domain/entities/player.h"
#include "domain/map/world.h"
#include "domain/minerals/coal_rock.h"

namespace {

template <typename T>
std::vector<T> only(const std::vector<ServerMessage>& messages) {
    std::vector<T> out;
    for (const ServerMessage& message: messages) {
        if (const T* found = std::get_if<T>(&message)) {
            out.push_back(*found);
        }
    }
    return out;
}

// Jugador 1 en (1,1) mirando a la derecha, carbón en (2,1) (se rompe de un golpe).
void populate(World& world) {
    world.add_player(1, std::make_unique<Player>(1, 1, 1));
    world.set_entity_at(2, 1, std::make_unique<CoalRock>(2, 1));
    world.set_player_facing(1, Direction::RIGHT);
}

TEST(WorldViewTest, FirstSyncSendsStatusCellsAndInventoryWithoutObtained) {
    World world(5, 5);
    populate(world);
    WorldView view;

    const std::vector<ServerMessage> messages = view.sync(world, 1);

    EXPECT_EQ(only<ServerStatus>(messages).size(), 1u);
    const auto resources = only<ServerResource>(messages);
    ASSERT_EQ(resources.size(), 1u);
    EXPECT_EQ(resources[0].x, 2);
    EXPECT_EQ(resources[0].y, 1);
    EXPECT_EQ(resources[0].content_id, "coal");
    EXPECT_EQ(resources[0].damage, ResourceDamage::Intact);
    EXPECT_EQ(only<ServerInventorySlot>(messages).size(), 4u);
    // El inventario inicial no es una obtención.
    EXPECT_TRUE(only<ServerObtained>(messages).empty());
}

TEST(WorldViewTest, SyncWithoutChangesSendsNothing) {
    World world(5, 5);
    populate(world);
    WorldView view;
    view.sync(world, 1);

    EXPECT_TRUE(view.sync(world, 1).empty());
}

TEST(WorldViewTest, ForgottenPlayerGetsTheWholeWorldAgain) {
    World world(5, 5);
    populate(world);
    WorldView view;
    const std::size_t first = view.sync(world, 1).size();

    view.forget(1);

    EXPECT_EQ(view.sync(world, 1).size(), first);
}

TEST(WorldViewTest, BlockedMoveTowardsRockChangesNothingVisible) {
    World world(5, 5);
    populate(world);
    WorldView view;
    view.sync(world, 1);

    // La roca bloquea el paso y el jugador ya mira hacia ella: nada cambia.
    world.move_player(1, Direction::RIGHT);
    EXPECT_TRUE(view.sync(world, 1).empty());
}

TEST(WorldViewTest, MiningRemovesRockAndShowsTheDropInItsPlace) {
    World world(5, 5);
    populate(world);
    WorldView view;
    view.sync(world, 1);

    world.use_tool(1);
    const std::vector<ServerMessage> messages = view.sync(world, 1);

    const auto removed = only<ServerResourceRemoved>(messages);
    ASSERT_EQ(removed.size(), 1u);
    EXPECT_EQ(removed[0].x, 2);
    EXPECT_EQ(removed[0].y, 1);
    const auto dropped = only<ServerDropped>(messages);
    ASSERT_EQ(dropped.size(), 1u);
    EXPECT_EQ(dropped[0].content_id, "coal");
    EXPECT_EQ(dropped[0].x, 2);
    EXPECT_EQ(dropped[0].y, 1);
}

TEST(WorldViewTest, WalkingOntoTheDropPicksItUpAndNotifiesThatPlayerOnly) {
    World world(5, 5);
    populate(world);
    world.add_player(2, std::make_unique<Player>(2, 4, 4));
    WorldView view;
    view.sync(world, 1);
    view.sync(world, 2);

    world.use_tool(1);
    view.sync(world, 1);
    view.sync(world, 2);
    world.move_player(1, Direction::RIGHT);

    const std::vector<ServerMessage> own = view.sync(world, 1);
    EXPECT_EQ(only<ServerDroppedRemoved>(own).size(), 1u);
    const auto slots = only<ServerInventorySlot>(own);
    ASSERT_EQ(slots.size(), 1u);
    EXPECT_EQ(slots[0].slot, 4);
    EXPECT_EQ(slots[0].count, 1);
    EXPECT_EQ(slots[0].content_id, "coal");
    const auto obtained = only<ServerObtained>(own);
    ASSERT_EQ(obtained.size(), 1u);
    EXPECT_EQ(obtained[0].content_id, "coal");

    const std::vector<ServerMessage> other = view.sync(world, 2);
    EXPECT_EQ(only<ServerDroppedRemoved>(other).size(), 1u);
    EXPECT_TRUE(only<ServerInventorySlot>(other).empty());
    EXPECT_TRUE(only<ServerObtained>(other).empty());
}

TEST(WorldViewTest, StackedPickupRaisesCountAndNotifiesAgain) {
    World world(5, 5);
    populate(world);
    world.add_player(2, std::make_unique<Player>(2, 4, 4));
    world.set_entity_at(3, 1, std::make_unique<CoalRock>(3, 1));
    WorldView view;
    view.sync(world, 1);

    world.use_tool(1);
    world.move_player(1, Direction::RIGHT);
    view.sync(world, 1);

    world.use_tool(1);
    world.move_player(1, Direction::RIGHT);
    const std::vector<ServerMessage> messages = view.sync(world, 1);

    const auto slots = only<ServerInventorySlot>(messages);
    ASSERT_EQ(slots.size(), 1u);
    EXPECT_EQ(slots[0].slot, 4);
    EXPECT_EQ(slots[0].count, 2);
    EXPECT_EQ(only<ServerObtained>(messages).size(), 1u);
}

}  // namespace
