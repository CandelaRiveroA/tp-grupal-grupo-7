#include "game/InteractionSession.h"

#include <optional>
#include <vector>

#include "gtest/gtest.h"

#include "world/CreatureCatalog.h"

namespace {

using game::InteractionSession;
using game::InteractionTarget;
using game::LocalWorldView;
using game::ViewEntity;
using world::ActionData;
using world::ActionKind;
using world::CreatureData;
using world::Direction;
using world::ItemKind;

ActionData actionWithFrames(int frames, int frameTicks) {
    ActionData data;
    data.frames = std::vector<world::Pose>(static_cast<std::size_t>(frames));
    data.frameTicks = frameTicks;
    return data;
}

CreatureData creatureWith(ActionKind action, int frames, int frameTicks) {
    CreatureData creature{};
    creature.directions[0].actions[action] =
            actionWithFrames(frames, frameTicks);
    return creature;
}

InteractionTarget milkTarget() {
    InteractionTarget target;
    target.action = ActionKind::Milk;
    target.cycles = 2;
    target.reward = "milk";
    target.cellX = 6;
    target.cellY = 5;
    return target;
}

TEST(InteractionSessionTest, StartsInactive) {
    InteractionSession session;
    EXPECT_FALSE(session.active());
    EXPECT_FALSE(session.advance());
}

TEST(InteractionSessionTest, DurationIsCyclesTimesActionData) {
    // 2 poses x 6 ticks = 12 por ciclo; 2 vueltas = 24 ticks, medidos
    // con los MISMOS datos que usa la animación (contrato §10).
    const CreatureData creature = creatureWith(ActionKind::Milk, 2, 6);
    InteractionSession session;
    session.start(milkTarget(), 5, 5, creature);

    int ticks = 0;
    bool closedOnLastTick = false;
    while (session.active()) {
        closedOnLastTick = session.advance();
        ++ticks;
    }
    EXPECT_EQ(ticks, 24);
    EXPECT_TRUE(closedOnLastTick);
    EXPECT_FALSE(session.advance());
}

TEST(InteractionSessionTest, WithoutActionDataTheWindowIsOneTick) {
    const CreatureData creature{};
    InteractionSession session;
    session.start(milkTarget(), 5, 5, creature);
    EXPECT_TRUE(session.active());
    EXPECT_TRUE(session.advance());
    EXPECT_FALSE(session.active());
}

TEST(InteractionSessionTest, PlayerAnimatesTheActionTowardTheTarget) {
    const CreatureData creature = creatureWith(ActionKind::Pickup, 4, 12);
    InteractionTarget target = milkTarget();
    target.action = ActionKind::Pickup;
    target.held = ItemKind::Egg;
    InteractionSession session;
    session.start(target, 5, 5, creature);  // objetivo en (6, 5)

    ViewEntity player;
    player.action = ActionKind::Idle;
    session.mergePlayer(player);
    EXPECT_EQ(player.action, ActionKind::Pickup);
    EXPECT_EQ(player.facing, Direction::Right);  // hacia el objetivo
    ASSERT_TRUE(player.activeTool.has_value());
    EXPECT_EQ(*player.activeTool, ItemKind::Egg);
}

TEST(InteractionSessionTest, FacingStaysAfterClosingUntilSomethingElse) {
    const CreatureData creature = creatureWith(ActionKind::Milk, 2, 6);
    InteractionSession session;
    session.start(milkTarget(), 5, 5, creature);

    ViewEntity player;
    while (session.active()) {
        session.advance();
    }
    // Terminó y el jugador está quieto: sigue mirando al objetivo.
    player.action = ActionKind::Idle;
    session.mergePlayer(player);
    EXPECT_EQ(player.facing, Direction::Right);

    // Si hace otra cosa, la fuente recupera el control del facing.
    player.action = ActionKind::Walk;
    session.mergePlayer(player);
    player.facing = Direction::Front;
    session.mergePlayer(player);
    EXPECT_EQ(player.facing, Direction::Front);
}

TEST(InteractionSessionTest, RewardsAreDeliveredOnceAndStack) {
    const CreatureData creature = creatureWith(ActionKind::Milk, 2, 6);
    InteractionSession session;
    session.start(milkTarget(), 5, 5, creature);

    // Cada merge arma una vista FRESCA (como hace la capa del mundo)
    // y la sesión re-aplica sus entregas: el evento dura un tick y el
    // inventario se mantiene.
    std::size_t obtainedTotal = 0;
    LocalWorldView last;
    while (session.active()) {
        session.advance();
        LocalWorldView view;
        session.mergeRewards(view);
        obtainedTotal += view.obtained.size();
        last = view;
    }
    EXPECT_EQ(obtainedTotal, 1u);
    ASSERT_EQ(last.inventory.size(), 1u);
    EXPECT_EQ(last.inventory[0].contentId, "milk");
    EXPECT_EQ(last.inventory[0].count, 1);

    // Un tick más (otra vista fresca): el evento no se repite y el
    // inventario se mantiene con su contador.
    LocalWorldView next;
    session.mergeRewards(next);
    EXPECT_TRUE(next.obtained.empty());
    ASSERT_EQ(next.inventory.size(), 1u);
    EXPECT_EQ(next.inventory[0].count, 1);
}

TEST(InteractionSessionTest, GainedItemsKeepTheirSlotsInAcquisitionOrder) {
    InteractionSession session;

    // Primero la leche: queda en el primer slot.
    session.start(milkTarget(), 5, 5,
                  creatureWith(ActionKind::Milk, 2, 6));
    while (session.active()) {
        session.advance();
    }

    // Después el huevo: entra al slot libre, SIN mover la leche.
    InteractionTarget egg = milkTarget();
    egg.action = ActionKind::Pickup;
    egg.reward = "egg";
    session.start(egg, 5, 5, creatureWith(ActionKind::Pickup, 4, 12));
    while (session.active()) {
        session.advance();
    }

    // Merge tras merge: manda el orden de obtención, no el alfabético
    // ("egg" < "milk" no debe robarle el cuadrado a la leche).
    for (int merge = 0; merge < 3; ++merge) {
        LocalWorldView view;
        session.mergeRewards(view);
        ASSERT_EQ(view.inventory.size(), 2u);
        EXPECT_EQ(view.inventory[0].contentId, "milk");
        EXPECT_EQ(view.inventory[0].slotIndex, 0);
        EXPECT_EQ(view.inventory[1].contentId, "egg");
        EXPECT_EQ(view.inventory[1].slotIndex, 1)
                << "en el merge " << merge;
    }
}

TEST(InteractionSessionTest, SameContentStacksInItsOwnSlot) {
    InteractionSession session;
    for (int round = 0; round < 2; ++round) {
        session.start(milkTarget(), 5, 5,
                      creatureWith(ActionKind::Milk, 2, 6));
        while (session.active()) {
            session.advance();
        }
    }
    LocalWorldView view;
    session.mergeRewards(view);
    ASSERT_EQ(view.inventory.size(), 1u);
    EXPECT_EQ(view.inventory[0].contentId, "milk");
    EXPECT_EQ(view.inventory[0].slotIndex, 0);
    EXPECT_EQ(view.inventory[0].count, 2);
}

TEST(InteractionSessionTest, GainRehomesWhenAnotherSourceTakesItsSlot) {
    InteractionSession session;
    session.start(milkTarget(), 5, 5, creatureWith(ActionKind::Milk, 2, 6));
    while (session.active()) {
        session.advance();
    }

    // Otra fuente (el inner) ocupa el slot 0 antes del merge: la
    // entrega se reubica en el primero libre.
    LocalWorldView view;
    view.addInventoryContent("silver");
    session.mergeRewards(view);
    ASSERT_EQ(view.inventory.size(), 2u);
    EXPECT_EQ(view.inventory[0].contentId, "silver");
    EXPECT_EQ(view.inventory[0].slotIndex, 0);
    EXPECT_EQ(view.inventory[1].contentId, "milk");
    EXPECT_EQ(view.inventory[1].slotIndex, 1);
}

}  // namespace
