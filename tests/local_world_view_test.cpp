#include <cstdint>

#include "common/common_command.h"
#include "game/GameClient.h"
#include "game/LocalWorldView.h"
#include "game/MockGameClient.h"
#include "gtest/gtest.h"
#include "world/CreatureCatalog.h"


namespace {

// Un tick del cliente: avanza el estado y devuelve la vista.
const game::LocalWorldView& tick(game::GameClient& client) {
    client.update(0);
    return client.worldState();
}

using game::GameClient;
using game::LocalWorldView;
using game::ViewEntity;

game::MockGameClient buildClient(int columns, int rows, int startX, int startY) {
    constexpr int kToolTicks = 30;
    return game::MockGameClient(columns, rows, startX, startY, kToolTicks);
}

protocol::ClientMessage walk(world::Direction direction) {
    return protocol::MoveMessage{world::wireDirectionFrom(direction)};
}

bool isValidDirection(world::Direction d) {
    switch (d) {
        case world::Direction::Front:
        case world::Direction::Back:
        case world::Direction::Left:
        case world::Direction::Right:
            return true;
    }
    return false;
}

bool isValidKind(world::CreatureKind k) {
    switch (k) {
        case world::CreatureKind::Player:
        case world::CreatureKind::Chicken:
        case world::CreatureKind::Cow:
        case world::CreatureKind::Orc:
        case world::CreatureKind::Skeleton:
        case world::CreatureKind::Vampire:
            return true;
    }
    return false;
}

TEST(LocalWorldViewContract, EntityZeroIsTheLocalPlayer) {
    game::MockGameClient concrete = buildClient(10, 10, 5, 5);
    GameClient& client = concrete;

    const LocalWorldView& view = tick(client);
    ASSERT_FALSE(view.entities.empty());
    EXPECT_EQ(view.entities[0].kind, world::CreatureKind::Player);
    EXPECT_EQ(view.entities[0].cellX, 5);
    EXPECT_EQ(view.entities[0].cellY, 5);
}

TEST(LocalWorldViewContract, EntityIndicesAreStableAcrossFrames) {
    game::MockGameClient concrete = buildClient(10, 10, 5, 5);
    GameClient& client = concrete;

    const std::size_t before = tick(client).entities.size();
    client.sendCommand(walk(world::Direction::Right));
    const std::size_t after = tick(client).entities.size();

    EXPECT_EQ(before, after);
    EXPECT_EQ(tick(client).entities[0].kind, world::CreatureKind::Player);
}

TEST(LocalWorldViewContract, MovingIsTrueOnAcceptedStep) {
    game::MockGameClient concrete = buildClient(10, 10, 5, 5);
    GameClient& client = concrete;

    client.sendCommand(walk(world::Direction::Right));
    EXPECT_TRUE(tick(client).entities[0].moving);
}

TEST(LocalWorldViewContract, MovingIsFalseWhenStepIsRejectedByTheEdge) {
    game::MockGameClient concrete = buildClient(10, 10, 9, 9);
    GameClient& client = concrete;

    client.sendCommand(walk(world::Direction::Right));
    EXPECT_FALSE(tick(client).entities[0].moving);
}

TEST(LocalWorldViewContract, MovingIsFalseWhenClampedAgainstTheSameCell) {
    game::MockGameClient concrete = buildClient(10, 10, 0, 0);
    GameClient& client = concrete;

    client.sendCommand(walk(world::Direction::Left));
    EXPECT_FALSE(tick(client).entities[0].moving);
}

TEST(LocalWorldViewContract, MovingIsFalseWhenTargetClampsBackToCurrentCell) {
    game::MockGameClient concrete = buildClient(10, 10, 9, 9);
    GameClient& client = concrete;

    client.sendCommand(walk(world::Direction::Right));
    const ViewEntity& player = tick(client).entities[0];
    EXPECT_EQ(player.cellX, 9);
    EXPECT_EQ(player.cellY, 9);
    EXPECT_FALSE(player.moving);
}

TEST(LocalWorldViewContract, CellStaysInsideTheGrid) {
    game::MockGameClient concrete = buildClient(10, 10, 5, 5);
    GameClient& client = concrete;

    for (int i = 0; i < 20; ++i) {
        client.sendCommand(walk(world::Direction::Right));
    }
    const ViewEntity& player = tick(client).entities[0];
    EXPECT_GE(player.cellX, 0);
    EXPECT_LT(player.cellX, 10);
    EXPECT_GE(player.cellY, 0);
    EXPECT_LT(player.cellY, 10);
}

TEST(LocalWorldViewContract, FacingAndKindAreAlwaysValid) {
    game::MockGameClient concrete = buildClient(10, 10, 5, 5);
    GameClient& client = concrete;

    const ViewEntity& initial = tick(client).entities[0];
    EXPECT_TRUE(isValidDirection(initial.facing));
    EXPECT_TRUE(isValidKind(initial.kind));

    client.sendCommand(walk(world::Direction::Right));
    const ViewEntity& moved = tick(client).entities[0];
    EXPECT_TRUE(isValidDirection(moved.facing));
    EXPECT_TRUE(isValidKind(moved.kind));
}

TEST(LocalWorldViewContract, ViewIsStableUntilNextCommand) {
    game::MockGameClient concrete = buildClient(10, 10, 5, 5);
    GameClient& client = concrete;

    client.sendCommand(walk(world::Direction::Right));
    const LocalWorldView& first = tick(client);
    const int x = first.entities[0].cellX;
    const int y = first.entities[0].cellY;
    const bool moving = first.entities[0].moving;
    const world::Direction facing = first.entities[0].facing;

    const LocalWorldView& second = client.worldState();
    ASSERT_FALSE(second.entities.empty());
    EXPECT_EQ(second.entities[0].cellX, x);
    EXPECT_EQ(second.entities[0].cellY, y);
    EXPECT_EQ(second.entities[0].moving, moving);
    EXPECT_EQ(second.entities[0].facing, facing);
}

TEST(LocalWorldViewContract, DroppedItemsAreEmptyByDefault) {
    LocalWorldView view;
    EXPECT_TRUE(view.droppedItems.empty());
}

TEST(LocalWorldViewContract, LocalPlayerCarriesStableIdZero) {
    game::MockGameClient concrete = buildClient(10, 10, 5, 5);
    GameClient& client = concrete;

    EXPECT_EQ(client.worldState().entities[0].id, 0);
    client.sendCommand(walk(world::Direction::Right));
    EXPECT_EQ(client.worldState().entities[0].id, 0);
}

TEST(LocalWorldViewContract, DroppedItemCarriesContentIdAndCell) {
    LocalWorldView view;
    view.droppedItems.push_back(game::ViewDroppedItem{"gold", 3, 7});
    ASSERT_EQ(view.droppedItems.size(), 1u);
    EXPECT_EQ(view.droppedItems[0].contentId, "gold");
    EXPECT_EQ(view.droppedItems[0].cellX, 3);
    EXPECT_EQ(view.droppedItems[0].cellY, 7);
}

TEST(LocalWorldViewContract, InventoryIsEmptyByDefault) {
    LocalWorldView view;
    EXPECT_TRUE(view.inventory.empty());
}

TEST(LocalWorldViewContract, InventorySlotCarriesContentIdAndCount) {
    LocalWorldView view;
    view.inventory.push_back(game::ViewInventorySlot{2, "silver", 21});
    ASSERT_EQ(view.inventory.size(), 1u);
    EXPECT_EQ(view.inventory[0].slotIndex, 2);
    EXPECT_EQ(view.inventory[0].contentId, "silver");
    EXPECT_EQ(view.inventory[0].count, 21);
}

TEST(LocalWorldViewContract, AddInventoryContentReturnsTheAssignedSlot) {
    LocalWorldView view;
    EXPECT_EQ(view.addInventoryContent("milk"), 0);
    EXPECT_EQ(view.addInventoryContent("egg"), 1);
    // Apila en el MISMO slot y lo devuelve.
    EXPECT_EQ(view.addInventoryContent("milk"), 0);
    ASSERT_EQ(view.inventory.size(), 2u);
    EXPECT_EQ(view.inventory[0].count, 2);
    EXPECT_EQ(view.inventory[1].count, 1);
}

TEST(LocalWorldViewContract, PlaceInventoryContentKeepsTheSlot) {
    LocalWorldView view;
    view.placeInventoryContent(3, "milk", 2);
    ASSERT_EQ(view.inventory.size(), 1u);
    EXPECT_EQ(view.inventory[0].slotIndex, 3);
    EXPECT_EQ(view.inventory[0].count, 2);
    // Otra vez: actualiza el conteo sin cambiar de slot.
    view.placeInventoryContent(3, "milk", 5);
    ASSERT_EQ(view.inventory.size(), 1u);
    EXPECT_EQ(view.inventory[0].slotIndex, 3);
    EXPECT_EQ(view.inventory[0].count, 5);
}

TEST(LocalWorldViewContract, InventorySlotFreeKnowsOccupiedSlots) {
    LocalWorldView view;
    EXPECT_TRUE(view.inventorySlotFree(0));
    view.addInventoryContent("milk");
    EXPECT_FALSE(view.inventorySlotFree(0));
    EXPECT_TRUE(view.inventorySlotFree(1));
}

}  // namespace
