#include <optional>
#include <string>
#include <vector>

#include "common/common_command.h"
#include "game/MockGameClient.h"
#include "gtest/gtest.h"
#include "world/CreatureCatalog.h"
#include "world/ViewDirection.h"

namespace {

// Un tick del cliente: avanza el estado y devuelve la vista.
const game::LocalWorldView& tick(game::GameClient& client) {
    client.update();
    return client.worldState();
}

// Duración del golpe (en consultas de estado) que el mock simula.
constexpr int kToolTicks = 30;

protocol::ClientMessage walk(world::Direction direction) {
    return protocol::MoveMessage{world::wireDirectionFrom(direction)};
}

protocol::ClientMessage pick(world::Direction direction) {
    return protocol::PickMessage{world::wireDirectionFrom(direction)};
}

protocol::ClientMessage take(world::Direction direction) {
    return protocol::TakeMessage{world::wireDirectionFrom(direction)};
}

TEST(MockGameClientTest, StartsWithLocalPlayerEntity) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    const game::LocalWorldView& view = tick(client);
    ASSERT_EQ(view.entities.size(), 1u);
    EXPECT_EQ(view.entities[0].cellX, 5);
    EXPECT_EQ(view.entities[0].cellY, 5);
}

TEST(MockGameClientTest, WalkMovesOneCellInDirection) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    client.sendCommand(walk(world::Direction::Right));
    const game::ViewEntity& player = tick(client).entities[0];
    EXPECT_EQ(player.cellX, 6);
    EXPECT_EQ(player.cellY, 5);
    EXPECT_EQ(player.facing, world::Direction::Right);
    EXPECT_TRUE(player.moving);
}

TEST(MockGameClientTest, WalkAccumulatesPosition) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    client.sendCommand(walk(world::Direction::Left));
    client.sendCommand(walk(world::Direction::Front));
    const game::ViewEntity& player = tick(client).entities[0];
    EXPECT_EQ(player.cellX, 4);
    EXPECT_EQ(player.cellY, 6);
}

TEST(MockGameClientTest, WalkClampsAtGridEdge) {
    game::MockGameClient client(10, 10, 9, 9, kToolTicks);
    client.sendCommand(walk(world::Direction::Right));
    const game::ViewEntity& player = tick(client).entities[0];
    EXPECT_EQ(player.cellX, 9);
    EXPECT_EQ(player.cellY, 9);
    EXPECT_FALSE(player.moving);
}

TEST(MockGameClientTest, WalkIntoEdgeKeepsFacingButNotMoving) {
    game::MockGameClient client(10, 10, 0, 0, kToolTicks);
    client.sendCommand(walk(world::Direction::Left));
    const game::ViewEntity& player = tick(client).entities[0];
    EXPECT_FALSE(player.moving);
    EXPECT_EQ(player.facing, world::Direction::Left);
}

TEST(MockGameClientTest, PickDoesNotMove) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    client.sendCommand(pick(world::Direction::Left));
    const game::ViewEntity& player = tick(client).entities[0];
    EXPECT_EQ(player.cellX, 5);
    EXPECT_EQ(player.cellY, 5);
    EXPECT_FALSE(player.moving);
    EXPECT_EQ(player.facing, world::Direction::Left);
}

TEST(MockGameClientTest, PlayerEntityCarriesItsKind) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    const game::ViewEntity& player = tick(client).entities[0];
    EXPECT_EQ(player.kind, world::CreatureKind::Player);
}

TEST(MockGameClientTest, ProvidesInitialInventoryTools) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    const std::vector<game::ViewInventorySlot>& inv = tick(client).inventory;
    ASSERT_EQ(inv.size(), 4u);
    EXPECT_EQ(inv[0].slotIndex, 0);
    EXPECT_EQ(inv[0].contentId, "pickaxe");
    EXPECT_EQ(inv[0].count, 1);
    EXPECT_EQ(inv[1].slotIndex, 1);
    EXPECT_EQ(inv[1].contentId, "shovel");
    EXPECT_EQ(inv[2].slotIndex, 2);
    EXPECT_EQ(inv[2].contentId, "watering_can");
    EXPECT_EQ(inv[3].slotIndex, 3);
    EXPECT_EQ(inv[3].contentId, "sword");
}

TEST(MockGameClientTest, InventoryStaysWhenPlayerMoves) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    client.sendCommand(walk(world::Direction::Right));
    EXPECT_EQ(tick(client).inventory.size(), 4u);
}

TEST(MockGameClientTest, WorldStateReportsGridDimensions) {
    game::MockGameClient client(10, 7, 5, 5, kToolTicks);
    const game::LocalWorldView& view = tick(client);
    EXPECT_EQ(view.grid.columns, 10);
    EXPECT_EQ(view.grid.rows, 7);
}

TEST(MockGameClientTest, ActiveToolIsNulloptInitially) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    const game::ViewEntity& player = tick(client).entities[0];
    EXPECT_FALSE(player.activeTool.has_value());
}

TEST(MockGameClientTest, ActiveToolIsPickaxeWhenUsingTool) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    client.sendCommand(pick(world::Direction::Back));
    const game::ViewEntity& player = tick(client).entities[0];
    ASSERT_TRUE(player.activeTool.has_value());
    EXPECT_EQ(*player.activeTool, world::ItemKind::Pickaxe);
    EXPECT_EQ(player.action, world::ActionKind::UseTool);
}

TEST(MockGameClientTest, ReturnsToIdleAfterToolUseDuration) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    client.sendCommand(pick(world::Direction::Back));
    // Consumir toda la ventana del golpe (una consulta por tick).
    for (int i = 0; i < kToolTicks; ++i) {
        tick(client);
    }
    const game::ViewEntity& player = tick(client).entities[0];
    EXPECT_EQ(player.action, world::ActionKind::Idle);
    EXPECT_FALSE(player.activeTool.has_value());
    // El facing se mantiene hacia donde picó.
    EXPECT_EQ(player.facing, world::Direction::Back);
}

TEST(MockGameClientTest, ActiveToolClearsAfterWalking) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    client.sendCommand(pick(world::Direction::Back));
    client.sendCommand(walk(world::Direction::Right));
    const game::ViewEntity& player = tick(client).entities[0];
    EXPECT_FALSE(player.activeTool.has_value());
}

// Busca un mineral por celda en la vista; devuelve su estado, o nullopt
// si en esa celda no hay (o se agotó).
std::optional<world::ResourceState> stateAt(const game::MockGameClient& client, int cellX,
                                            int cellY) {
    for (const game::ViewResource& r: client.worldState().resources) {
        // cppcheck-suppress useStlAlgorithm
        if (r.cellX == cellX && r.cellY == cellY) {
            return r.state;
        }
    }
    return std::nullopt;
}

TEST(MockGameClientTest, StartsWithSeveralIntactMinerals) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    const std::vector<game::ViewResource>& res = tick(client).resources;
    // Hay varias piedras y todas arrancan intactas.
    EXPECT_GT(res.size(), 1u);
    for (const game::ViewResource& r: res) {
        EXPECT_EQ(r.state, world::ResourceState::Intact);
    }
    // Una conocida: la plata a la derecha del jugador (6,5).
    EXPECT_EQ(stateAt(client, 6, 5), world::ResourceState::Intact);
}

// Consume la ventana del golpe (una consulta por tick) para que el mock
// confirme el daño al final, como hará el servidor.
void completeHit(game::MockGameClient& client) {
    for (int i = 0; i < kToolTicks; ++i) {
        tick(client);
    }
}

TEST(MockGameClientTest, DamageAppliesOnlyAfterCompletingTheHit) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    client.sendCommand(pick(world::Direction::Right));
    // Apenas apretado, el mineral sigue intacto (el daño no es inmediato).
    EXPECT_EQ(stateAt(client, 6, 5), world::ResourceState::Intact);
    completeHit(client);
    EXPECT_EQ(stateAt(client, 6, 5), world::ResourceState::Cracked);
}

TEST(MockGameClientTest, PickingMineralAdvancesItsState) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    client.sendCommand(pick(world::Direction::Right));
    completeHit(client);
    EXPECT_EQ(stateAt(client, 6, 5), world::ResourceState::Cracked);
    client.sendCommand(pick(world::Direction::Right));
    completeHit(client);
    EXPECT_EQ(stateAt(client, 6, 5), world::ResourceState::Broken);
}

TEST(MockGameClientTest, MineralDisappearsAfterThirdHit) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    for (int hit = 0; hit < 3; ++hit) {
        client.sendCommand(pick(world::Direction::Right));
        completeHit(client);
    }
    // La piedra golpeada desaparece, pero las otras siguen.
    EXPECT_FALSE(stateAt(client, 6, 5).has_value());
    EXPECT_FALSE(tick(client).resources.empty());
}

TEST(MockGameClientTest, PickingOneMineralLeavesOthersIntact) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    client.sendCommand(pick(world::Direction::Right));
    completeHit(client);
    // La golpeada avanzó, pero otra piedra (ej. 7,5) sigue intacta.
    EXPECT_EQ(stateAt(client, 6, 5), world::ResourceState::Cracked);
    EXPECT_EQ(stateAt(client, 7, 5), world::ResourceState::Intact);
}

// Agota la piedra frente al jugador con 3 golpes completos.
void exhaustFacingMineral(game::MockGameClient& client, world::Direction direction) {
    for (int hit = 0; hit < 3; ++hit) {
        client.sendCommand(pick(direction));
        completeHit(client);
    }
}

// Busca un dropped item por celda; devuelve su contentId o nullopt.
std::optional<std::string> droppedAt(const game::MockGameClient& client, int cellX, int cellY) {
    for (const game::ViewDroppedItem& d: client.worldState().droppedItems) {
        // cppcheck-suppress useStlAlgorithm
        if (d.cellX == cellX && d.cellY == cellY) {
            return d.contentId;
        }
    }
    return std::nullopt;
}

TEST(MockGameClientTest, ExhaustingMineralDropsItemOnTheFloor) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    // Antes de agotar no hay nada en el piso.
    EXPECT_FALSE(droppedAt(client, 6, 5).has_value());
    exhaustFacingMineral(client, world::Direction::Right);
    // Al agotarse, el item cae en su celda con su contentId genérico.
    EXPECT_EQ(droppedAt(client, 6, 5), "silver");
}

TEST(MockGameClientTest, ExhaustingMineralDoesNotEmitObtainedAutomatically) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    exhaustFacingMineral(client, world::Direction::Right);
    // Ya no hay obtención automática: hay que recoger con Take.
    for (int i = 0; i < 60; ++i) {
        EXPECT_TRUE(tick(client).obtained.empty());
    }
}

TEST(MockGameClientTest, TakeOnDroppedCellEmitsObtained) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    exhaustFacingMineral(client, world::Direction::Right);
    // El jugador se para sobre la celda del item y lo recoge.
    client.sendCommand(walk(world::Direction::Right));
    client.sendCommand(take(world::Direction::Right));
    const game::LocalWorldView& view = tick(client);
    ASSERT_EQ(view.obtained.size(), 1u);
    EXPECT_EQ(view.obtained.front().kind, world::ResourceKind::Silver);
    // El item desaparece del piso al recogerse.
    EXPECT_FALSE(droppedAt(client, 6, 5).has_value());
}

TEST(MockGameClientTest, TakeOnEmptyCellEmitsNothing) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    // No hay item dropeado todavía: recoger no emite nada.
    client.sendCommand(take(world::Direction::Front));
    EXPECT_TRUE(tick(client).obtained.empty());
}

TEST(MockGameClientTest, ObtainedFromTakeLastsOnlyOneTick) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    exhaustFacingMineral(client, world::Direction::Right);
    client.sendCommand(walk(world::Direction::Right));
    client.sendCommand(take(world::Direction::Right));
    // Primer estado: trae el evento.
    ASSERT_FALSE(tick(client).obtained.empty());
    // El tick siguiente ya no lo tiene (es puntual, no persistente).
    EXPECT_TRUE(tick(client).obtained.empty());
}

// Busca un slot del inventario por contentId; devuelve su count o nullopt.
std::optional<int> inventoryCount(const game::MockGameClient& client,
                                  const std::string& contentId) {
    for (const game::ViewInventorySlot& s: client.worldState().inventory) {
        // cppcheck-suppress useStlAlgorithm
        if (s.contentId == contentId) {
            return s.count;
        }
    }
    return std::nullopt;
}

TEST(MockGameClientTest, TakingMineralAddsItToInventoryWithCountOne) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    EXPECT_FALSE(inventoryCount(client, "silver").has_value());
    exhaustFacingMineral(client, world::Direction::Right);  // silver (6,5)
    client.sendCommand(walk(world::Direction::Right));
    client.sendCommand(take(world::Direction::Right));
    EXPECT_EQ(inventoryCount(client, "silver"), 1);
    // Las herramientas siguen presentes.
    EXPECT_EQ(inventoryCount(client, "pickaxe"), 1);
    EXPECT_EQ(inventoryCount(client, "shovel"), 1);
}

TEST(MockGameClientTest, TakingSameMineralTwiceAccumulatesCount) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    // Dos piedras de plata: (6,5) y (5,7).
    exhaustFacingMineral(client, world::Direction::Right);  // silver (6,5)
    client.sendCommand(walk(world::Direction::Right));
    client.sendCommand(take(world::Direction::Right));
    EXPECT_EQ(inventoryCount(client, "silver"), 1);

    // Volver a (5,5) -> bajar a (5,6) para quedar frente a (5,7).
    client.sendCommand(walk(world::Direction::Left));
    client.sendCommand(walk(world::Direction::Front));
    exhaustFacingMineral(client, world::Direction::Front);  // silver (5,7)
    client.sendCommand(walk(world::Direction::Front));
    client.sendCommand(take(world::Direction::Front));
    EXPECT_EQ(inventoryCount(client, "silver"), 2);
}

TEST(MockGameClientTest, TakingDifferentMineralsUsesDifferentSlots) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    exhaustFacingMineral(client, world::Direction::Right);  // silver (6,5)
    client.sendCommand(walk(world::Direction::Right));
    client.sendCommand(take(world::Direction::Right));
    exhaustFacingMineral(client, world::Direction::Right);  // coal (7,5)
    client.sendCommand(walk(world::Direction::Right));
    client.sendCommand(take(world::Direction::Right));
    EXPECT_EQ(inventoryCount(client, "silver"), 1);
    EXPECT_EQ(inventoryCount(client, "coal"), 1);
    // Slots distintos (índices libres tras las 4 herramientas).
    std::optional<int> silverSlot;
    std::optional<int> coalSlot;
    for (const game::ViewInventorySlot& s: tick(client).inventory) {
        if (s.contentId == "silver") {
            silverSlot = s.slotIndex;
        }
        if (s.contentId == "coal") {
            coalSlot = s.slotIndex;
        }
    }
    ASSERT_TRUE(silverSlot.has_value());
    ASSERT_TRUE(coalSlot.has_value());
    EXPECT_NE(*silverSlot, *coalSlot);
}

TEST(MockGameClientTest, WorldStateDoesNotAdvanceUntilUpdate) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    client.sendCommand(pick(world::Direction::Back));
    // Consultar la vista no avanza la animación: sigue usando la herramienta.
    for (int i = 0; i < kToolTicks; ++i) {
        client.worldState();
    }
    EXPECT_EQ(client.worldState().entities[0].action, world::ActionKind::UseTool);
    for (int i = 0; i < kToolTicks; ++i) {
        client.update();
    }
    EXPECT_EQ(client.worldState().entities[0].action, world::ActionKind::Idle);
}

TEST(MockGameClientTest, ObtainedIsEmittedOncePerUpdate) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    exhaustFacingMineral(client, world::Direction::Right);
    client.sendCommand(walk(world::Direction::Right));
    client.sendCommand(take(world::Direction::Right));
    client.update();
    EXPECT_EQ(client.worldState().obtained.size(), 1u);
    // Consultar varias veces sin update() no lo duplica ni lo borra.
    EXPECT_EQ(client.worldState().obtained.size(), 1u);
    client.update();
    EXPECT_TRUE(client.worldState().obtained.empty());
}

}  // namespace
