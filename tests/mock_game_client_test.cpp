#include <optional>
#include <string>
#include <vector>

#include "common/common_command.h"
#include "game/MockGameClient.h"
#include "gtest/gtest.h"
#include "world/CharacterCatalogLoader.h"
#include "world/CreatureCatalog.h"
#include "world/ViewDirection.h"

namespace {

// Un tick del cliente: avanza el estado y devuelve la vista.
const game::LocalWorldView& tick(game::GameClient& client) {
    client.update(0);
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
        client.update(0);
    }
    EXPECT_EQ(client.worldState().entities[0].action, world::ActionKind::Idle);
}

TEST(MockGameClientTest, ObtainedIsEmittedOncePerUpdate) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    exhaustFacingMineral(client, world::Direction::Right);
    client.sendCommand(walk(world::Direction::Right));
    client.sendCommand(take(world::Direction::Right));
    client.update(0);
    EXPECT_EQ(client.worldState().obtained.size(), 1u);
    // Consultar varias veces sin update() no lo duplica ni lo borra.
    EXPECT_EQ(client.worldState().obtained.size(), 1u);
    client.update(0);
    EXPECT_TRUE(client.worldState().obtained.empty());
}

// --- Animales: la simulación del "server falso" --------------------------

namespace {

// Catálogo mínimo (jugador + gallina + vaca) para probar el mundo con
// animales.
const char* const kWorldToml = R"(
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

[characters.chicken]
sheet   = "Chicken.png"
grid    = [16, 16]
drawn   = [32, 32]
cadence = { step = 32, frame = 8 }
  [characters.chicken.front]
  mirrored = false
    [characters.chicken.front.idle]
    loops  = false
    frames = [ [[0, 4]] ]
    [characters.chicken.front.walk]
    loops  = true
    frames = [ [[0, 0]], [[1, 0]], [[2, 0]], [[3, 0]] ]
  [characters.chicken.right]
  mirrored = false
    [characters.chicken.right.idle]
    loops  = false
    frames = [ [[2, 4]] ]
    [characters.chicken.right.walk]
    loops  = true
    frames = [ [[0, 1]], [[1, 1]], [[2, 1]], [[3, 1]] ]
  [characters.chicken.back]
  mirrored = false
    [characters.chicken.back.idle]
    loops  = false
    frames = [ [[0, 5]] ]
    [characters.chicken.back.walk]
    loops  = true
    frames = [ [[0, 2]], [[1, 2]], [[2, 2]], [[3, 2]] ]
  [characters.chicken.left]
  mirrored = false
    [characters.chicken.left.idle]
    loops  = false
    frames = [ [[2, 5]] ]
    [characters.chicken.left.walk]
    loops  = true
    frames = [ [[0, 3]], [[1, 3]], [[2, 3]], [[3, 3]] ]

[characters.cow]
sheet   = "Cow.png"
grid    = [32, 32]
drawn   = [64, 64]
cadence = { step = 32, frame = 32 }
  [characters.cow.front]
  mirrored = false
    [characters.cow.front.idle]
    loops  = false
    frames = [ [[0, 1]] ]
    [characters.cow.front.walk]
    loops  = true
    frames = [ [[0, 1]] ]
  [characters.cow.right]
  mirrored = false
    [characters.cow.right.idle]
    loops  = false
    frames = [ [[0, 1]] ]
    [characters.cow.right.walk]
    loops  = true
    frames = [ [[0, 1]] ]
  [characters.cow.back]
  mirrored = false
    [characters.cow.back.idle]
    loops  = false
    frames = [ [[0, 2]] ]
    [characters.cow.back.walk]
    loops  = true
    frames = [ [[0, 2]] ]
  [characters.cow.left]
  mirrored = false
    [characters.cow.left.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.cow.left.walk]
    loops  = true
    frames = [ [[0, 0]] ]
)";

// El spawn de los animales es dato del mock; para que las 3 celdas caigan
// dentro, el mundo de prueba usa la grilla de la sala real (30x20).
constexpr int kRoomColumns = 30;
constexpr int kRoomRows = 20;
constexpr int kPlayerX = 15;
constexpr int kPlayerY = 10;

const world::CharacterCatalog& worldCatalog() {
    static const world::CharacterCatalog instance =
            world::CharacterCatalogLoader().loadFromString(kWorldToml);
    return instance;
}

// Camina el mundo N ticks acumulando los ítems del suelo visibles en
// cada frame (el último estado manda).
std::vector<game::ViewDroppedItem> groundItemsAfter(
        game::MockGameClient& client, int lastIteration) {
    std::vector<game::ViewDroppedItem> last;
    for (int it = 0; it <= lastIteration; ++it) {
        client.update(it);
        last = client.worldState().droppedItems;
    }
    return last;
}

}

TEST(MockGameClientTest, SpawnsAnimalsWhenTheCatalogHasThem) {
    game::MockGameClient client(kRoomColumns, kRoomRows, kPlayerX,
                                kPlayerY, kToolTicks, worldCatalog());
    const game::LocalWorldView& view = client.worldState();
    // 3 gallinas + 3 vacas + jugador, en orden de spawn.
    ASSERT_EQ(view.entities.size(), 7u);

    // El jugador sigue siendo la entidad 0 con id 0.
    EXPECT_EQ(view.entities[0].kind, world::CreatureKind::Player);
    EXPECT_EQ(view.entities[0].id, 0);

    // El orden del estado replica el orden de spawn: gallinas y luego
    // vacas (el orden de la tabla de bindings del mock).
    for (std::size_t i = 1; i <= 3; ++i) {
        EXPECT_EQ(view.entities[i].kind, world::CreatureKind::Chicken);
    }
    for (std::size_t i = 4; i < view.entities.size(); ++i) {
        EXPECT_EQ(view.entities[i].kind, world::CreatureKind::Cow);
    }
    for (std::size_t i = 1; i < view.entities.size(); ++i) {
        EXPECT_EQ(view.entities[i].id, static_cast<int>(i));
        EXPECT_FALSE(view.entities[i].cellX == kPlayerX &&
                     view.entities[i].cellY == kPlayerY);
    }
}

TEST(MockGameClientTest, AnimalIdsAreUnique) {
    game::MockGameClient client(kRoomColumns, kRoomRows, kPlayerX,
                                kPlayerY, kToolTicks, worldCatalog());
    const game::LocalWorldView& view = client.worldState();
    for (std::size_t a = 0; a < view.entities.size(); ++a) {
        for (std::size_t b = a + 1; b < view.entities.size(); ++b) {
            EXPECT_NE(view.entities[a].id, view.entities[b].id);
        }
    }
}

TEST(MockGameClientTest, AnimalsMoveOverUpdateTicks) {
    game::MockGameClient client(kRoomColumns, kRoomRows, kPlayerX,
                                kPlayerY, kToolTicks, worldCatalog());

    int movedCells = 0;
    for (int it = 0; it <= 200; ++it) {
        const game::LocalWorldView& before = client.worldState();
        std::vector<int> xBefore;
        std::vector<int> yBefore;
        for (const game::ViewEntity& entity : before.entities) {
            xBefore.push_back(entity.cellX);
            yBefore.push_back(entity.cellY);
        }

        client.update(it);

        const game::LocalWorldView& after = client.worldState();
        ASSERT_EQ(after.entities.size(), before.entities.size());
        for (std::size_t i = 0; i < after.entities.size(); ++i) {
            if (after.entities[i].cellX != xBefore[i] ||
                after.entities[i].cellY != yBefore[i]) {
                movedCells += 1;
            }
            if (after.entities[i].kind == world::CreatureKind::Player) {
                continue;
            }
            // Nadie pisa al jugador, nadie sale de la grilla.
            EXPECT_FALSE(after.entities[i].cellX == kPlayerX &&
                         after.entities[i].cellY == kPlayerY);
            EXPECT_GE(after.entities[i].cellX, 0);
            EXPECT_LT(after.entities[i].cellX, kRoomColumns);
            EXPECT_GE(after.entities[i].cellY, 0);
            EXPECT_LT(after.entities[i].cellY, kRoomRows);
        }
    }
    EXPECT_GT(movedCells, 0);
}

TEST(MockGameClientTest, AnimalActionMatchesItsMovement) {
    game::MockGameClient client(kRoomColumns, kRoomRows, kPlayerX,
                                kPlayerY, kToolTicks, worldCatalog());
    for (int it = 0; it <= 200; ++it) {
        client.update(it);
        for (const game::ViewEntity& entity : client.worldState().entities) {
            if (entity.kind == world::CreatureKind::Player) {
                continue;
            }
            const world::ActionKind expected =
                    entity.moving ? world::ActionKind::Walk
                                  : world::ActionKind::Idle;
            EXPECT_EQ(entity.action, expected);
        }
    }
}

TEST(MockGameClientTest, UpdateWithoutAnimalSimulationIsANoOp) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    client.update(0);
    client.update(50);
    const game::LocalWorldView& view = client.worldState();
    ASSERT_EQ(view.entities.size(), 1u);
    EXPECT_EQ(view.entities[0].kind, world::CreatureKind::Player);
}

// --- Huevos: la gallina pone, el ítem aparece en el suelo ---------------

TEST(MockGameClientTest, ProducingAnimalsDropItemsOnTheGround) {
    game::MockGameClient client(kRoomColumns, kRoomRows, kPlayerX,
                                kPlayerY, kToolTicks, worldCatalog());

    // El período de postura es largo (gama en el .cpp de la
    // simulación) pero acotado: con esta seed la primera postura cae
    // bien dentro de la caminata.
    const std::vector<game::ViewDroppedItem> drops =
            groundItemsAfter(client, 3000);
    ASSERT_FALSE(drops.empty());

    for (const game::ViewDroppedItem& drop : drops) {
        // El ítem es uno de los que la tabla de postura declara para
        // algún kind (gallina -> huevo, vaca -> leche).
        const bool knownDrop = drop.contentId == "egg" || drop.contentId == "milk";
        EXPECT_TRUE(knownDrop);
        EXPECT_GE(drop.cellX, 0);
        EXPECT_LT(drop.cellX, kRoomColumns);
        EXPECT_GE(drop.cellY, 0);
        EXPECT_LT(drop.cellY, kRoomRows);
        // Nadie suelta un ítem en la celda del jugador.
        EXPECT_FALSE(drop.cellX == kPlayerX && drop.cellY == kPlayerY);
    }
}

TEST(MockGameClientTest, GroundItemCountNeverExceedsTheCap) {
    // Sin recogida, el tope de ítems en el suelo es lo que evita que
    // la vista se llene de ítems (regla silenciosa del "server
    // falso", como el límite de slots del inventario).
    game::MockGameClient client(kRoomColumns, kRoomRows, kPlayerX,
                                kPlayerY, kToolTicks, worldCatalog());
    const std::vector<game::ViewDroppedItem> drops =
            groundItemsAfter(client, 8000);
    EXPECT_LE(drops.size(), 3u);
}

TEST(MockGameClientTest, GroundItemsAreStableBetweenFrames) {
    game::MockGameClient client(kRoomColumns, kRoomRows, kPlayerX,
                                kPlayerY, kToolTicks, worldCatalog());
    const std::vector<game::ViewDroppedItem> drops =
            groundItemsAfter(client, 3000);
    ASSERT_FALSE(drops.empty());

    // 200 ticks más: los ítems de antes SIGUEN (los del suelo no
    // desaparecen), identificados por contentId + celda.
    const std::vector<game::ViewDroppedItem> later =
            groundItemsAfter(client, 3200);
    for (const game::ViewDroppedItem& drop : drops) {
        bool found = false;
        for (const game::ViewDroppedItem& candidate : later) {
            found = found || (candidate.contentId == drop.contentId &&
                              candidate.cellX == drop.cellX &&
                              candidate.cellY == drop.cellY);
        }
        EXPECT_TRUE(found) << drop.contentId << " desapareció";
    }
}

TEST(MockGameClientTest, GroundItemsNeverShareACell) {
    game::MockGameClient client(kRoomColumns, kRoomRows, kPlayerX,
                                kPlayerY, kToolTicks, worldCatalog());
    for (int it = 0; it <= 8000; ++it) {
        client.update(it);
        const std::vector<game::ViewDroppedItem>& drops =
                client.worldState().droppedItems;
        for (std::size_t a = 0; a < drops.size(); ++a) {
            for (std::size_t b = a + 1; b < drops.size(); ++b) {
                EXPECT_FALSE(drops[a].cellX == drops[b].cellX &&
                             drops[a].cellY == drops[b].cellY)
                        << "ítems " << drops[a].contentId << " y "
                        << drops[b].contentId << " comparten celda";
            }
        }
    }
}

TEST(MockGameClientTest, WorldWithoutAnimalSimulationHasNoGroundItems) {
    game::MockGameClient client(10, 10, 5, 5, kToolTicks);
    const std::vector<game::ViewDroppedItem> drops =
            groundItemsAfter(client, 3000);
    EXPECT_TRUE(drops.empty());
}

}  // namespace
