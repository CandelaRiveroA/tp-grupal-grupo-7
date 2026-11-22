#include <cstdlib>
#include <memory>
#include <variant>
#include <vector>

#include "common/common_command.h"
#include "game/MockWorldClient.h"
#include "game/GameClient.h"
#include "game/LocalWorldView.h"
#include "game/MockGameClient.h"
#include "gtest/gtest.h"
#include "world/CharacterCatalogLoader.h"
#include "world/CreatureCatalog.h"
#include "world/ViewDirection.h"

namespace {

using game::MockWorldClient;
using game::GameClient;
using game::LocalWorldView;
using game::ViewDroppedItem;
using world::CreatureKind;

// El mundo de prueba usa la grilla de la sala real (30x20) para que las
// celdas de spawn de los animales caigan adentro.
constexpr int kRoomColumns = 30;
constexpr int kRoomRows = 20;
constexpr int kPlayerX = 15;
constexpr int kPlayerY = 10;
constexpr int kToolTicks = 30;

// Catálogo real: trae al jugador, la gallina y la vaca.
const world::CharacterCatalog& worldCatalog() {
    static const world::CharacterCatalog instance =
            world::CharacterCatalogLoader().loadFromFile(
                    TALLER_CHARACTERS_TOML);
    return instance;
}

// El overlay sobre un mock: el mock es el "server" y el overlay agrega
// los animales, igual que en el juego (donde el inner es la red).
std::unique_ptr<GameClient> makeClient(
        const world::CharacterCatalog& characters) {
    return std::make_unique<MockWorldClient>(
            std::make_unique<game::MockGameClient>(
                    kRoomColumns, kRoomRows, kPlayerX, kPlayerY, kToolTicks),
            characters, world::GridSpan{kRoomColumns, kRoomRows});
}

// Camina el mundo N ticks acumulando los ítems del suelo visibles en
// cada frame (el último estado manda).
std::vector<ViewDroppedItem> groundItemsAfter(GameClient& client,
                                              int lastIteration) {
    std::vector<ViewDroppedItem> last;
    for (int it = 0; it <= lastIteration; ++it) {
        client.update(it);
        last = client.worldState().droppedItems;
    }
    return last;
}

protocol::ClientMessage walk(world::Direction direction) {
    return protocol::MoveMessage{world::wireDirectionFrom(direction)};
}

protocol::ClientMessage take(world::Direction direction) {
    return protocol::TakeMessage{world::wireDirectionFrom(direction), 0};
}

world::Direction toward(int fromX, int fromY, int toX, int toY) {
    const int dx = toX - fromX;
    const int dy = toY - fromY;
    if (std::abs(dx) >= std::abs(dy)) {
        return dx >= 0 ? world::Direction::Right : world::Direction::Left;
    }
    return dy >= 0 ? world::Direction::Front : world::Direction::Back;
}

TEST(MockWorldClientTest, SpawnsAnimalsWhenTheCatalogHasThem) {
    const std::unique_ptr<GameClient> client = makeClient(worldCatalog());
    const LocalWorldView& view = client->worldState();
    // 3 gallinas + 3 vacas + jugador, en orden de spawn. Los enemigos
    // viven en enemies.toml (este catálogo no lo carga), así que no
    // aparecen: AnimalSystem solo spawnea los kinds que el catálogo
    // describe.
    ASSERT_EQ(view.entities.size(), 7u);

    // El jugador sigue siendo la entidad 0 con id 0.
    EXPECT_EQ(view.entities[0].kind, CreatureKind::Player);
    EXPECT_EQ(view.entities[0].id, 0);

    // El orden del estado replica el orden de spawn: gallinas y luego
    // vacas (el orden de la tabla de bindings de la capa).
    for (std::size_t i = 1; i <= 3; ++i) {
        EXPECT_EQ(view.entities[i].kind, CreatureKind::Chicken);
    }
    for (std::size_t i = 4; i < view.entities.size(); ++i) {
        EXPECT_EQ(view.entities[i].kind, CreatureKind::Cow);
    }
    for (std::size_t i = 1; i < view.entities.size(); ++i) {
        EXPECT_EQ(view.entities[i].id, static_cast<int>(i));
        EXPECT_FALSE(view.entities[i].cellX == kPlayerX &&
                     view.entities[i].cellY == kPlayerY);
    }
}

TEST(MockWorldClientTest, WithoutAppearancesOnlyThePlayerShows) {
    const world::CharacterCatalog empty;
    const std::unique_ptr<GameClient> client = makeClient(empty);
    const LocalWorldView& view = client->worldState();
    ASSERT_EQ(view.entities.size(), 1u);
    EXPECT_EQ(view.entities[0].kind, CreatureKind::Player);
}

TEST(MockWorldClientTest, AnimalIdsAreUnique) {
    const std::unique_ptr<GameClient> client = makeClient(worldCatalog());
    const LocalWorldView& view = client->worldState();
    for (std::size_t a = 0; a < view.entities.size(); ++a) {
        for (std::size_t b = a + 1; b < view.entities.size(); ++b) {
            EXPECT_NE(view.entities[a].id, view.entities[b].id);
        }
    }
}

TEST(MockWorldClientTest, AnimalsMoveOverUpdateTicks) {
    const std::unique_ptr<GameClient> client = makeClient(worldCatalog());

    int movedCells = 0;
    for (int it = 0; it <= 3000; ++it) {
        const LocalWorldView& before = client->worldState();
        std::vector<int> xBefore;
        std::vector<int> yBefore;
        for (const game::ViewEntity& entity : before.entities) {
            xBefore.push_back(entity.cellX);
            yBefore.push_back(entity.cellY);
        }

        client->update(it);

        const LocalWorldView& after = client->worldState();
        ASSERT_EQ(after.entities.size(), before.entities.size());
        for (std::size_t i = 0; i < after.entities.size(); ++i) {
            if (after.entities[i].cellX != xBefore[i] ||
                after.entities[i].cellY != yBefore[i]) {
                movedCells += 1;
            }
            if (after.entities[i].kind == CreatureKind::Player) {
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

TEST(MockWorldClientTest, AnimalActionMatchesItsMovement) {
    const std::unique_ptr<GameClient> client = makeClient(worldCatalog());
    for (int it = 0; it <= 200; ++it) {
        client->update(it);
        for (const game::ViewEntity& entity : client->worldState().entities) {
            if (entity.kind == CreatureKind::Player) {
                continue;
            }
            const world::ActionKind expected =
                    entity.moving ? world::ActionKind::Walk
                                  : world::ActionKind::Idle;
            EXPECT_EQ(entity.action, expected);
        }
    }
}

TEST(MockWorldClientTest, CommandsReachTheInnerSource) {
    const std::unique_ptr<GameClient> client = makeClient(worldCatalog());

    // El jugador camina una celda a la derecha en el mock de adentro;
    // la vista fusionada lo muestra (con su id estable 0).
    client->sendCommand(walk(world::Direction::Right));
    client->update(0);
    const LocalWorldView& view = client->worldState();
    EXPECT_EQ(view.entities[0].cellX, kPlayerX + 1);
    EXPECT_EQ(view.entities[0].cellY, kPlayerY);
    EXPECT_EQ(view.entities[0].id, 0);
}

// --- Huevos: la gallina pone, el ítem aparece en el suelo ---------------

TEST(MockWorldClientTest, ProducingAnimalsDropItemsOnTheGround) {
    const std::unique_ptr<GameClient> client = makeClient(worldCatalog());

    // El período de postura es largo (gama en el .cpp de la
    // simulación) pero acotado: con esta seed la primera postura cae
    // bien dentro de la caminata.
    const std::vector<ViewDroppedItem> drops = groundItemsAfter(*client, 3000);
    ASSERT_FALSE(drops.empty());

    for (const ViewDroppedItem& drop : drops) {
        // Al piso caen huevos; la leche NO: la vaca queda lista para
        // ordeñar y lo dice con su diálogo.
        EXPECT_EQ(drop.contentId, "egg");
        // El "server falso" los marca interactivos (tocables).
        EXPECT_TRUE(drop.interactive);
        EXPECT_GE(drop.cellX, 0);
        EXPECT_LT(drop.cellX, kRoomColumns);
        EXPECT_GE(drop.cellY, 0);
        EXPECT_LT(drop.cellY, kRoomRows);
        // Nadie suelta un ítem en la celda del jugador.
        EXPECT_FALSE(drop.cellX == kPlayerX && drop.cellY == kPlayerY);
    }

    // Y al menos una vaca queda con su indicación de lista.
    bool anyReady = false;
    for (const game::ViewEntity& entity : client->worldState().entities) {
        if (entity.kind == CreatureKind::Cow &&
            entity.indication == world::Indication::ReadyToMilk) {
            anyReady = true;
        }
    }
    EXPECT_TRUE(anyReady);
}

TEST(MockWorldClientTest, GroundItemCountNeverExceedsTheCap) {
    // Sin recogida, el tope de ítems en el suelo es lo que evita que
    // la vista se llene de ítems (regla silenciosa del "server
    // falso", como el límite de slots del inventario).
    const std::unique_ptr<GameClient> client = makeClient(worldCatalog());
    const std::vector<ViewDroppedItem> drops = groundItemsAfter(*client, 8000);
    EXPECT_LE(drops.size(), 3u);
}

TEST(MockWorldClientTest, GroundItemsAreStableBetweenFrames) {
    const std::unique_ptr<GameClient> client = makeClient(worldCatalog());
    const std::vector<ViewDroppedItem> drops = groundItemsAfter(*client, 3000);
    ASSERT_FALSE(drops.empty());

    // 200 ticks más: los ítems de antes SIGUEN (los del suelo no
    // desaparecen), identificados por contentId + celda.
    const std::vector<ViewDroppedItem> later = groundItemsAfter(*client, 3200);
    for (const ViewDroppedItem& drop : drops) {
        bool found = false;
        for (const ViewDroppedItem& candidate : later) {
            found = found || (candidate.contentId == drop.contentId &&
                              candidate.cellX == drop.cellX &&
                              candidate.cellY == drop.cellY);
        }
        EXPECT_TRUE(found) << drop.contentId << " desapareció";
    }
}

TEST(MockWorldClientTest, GroundItemsNeverShareACell) {
    const std::unique_ptr<GameClient> client = makeClient(worldCatalog());
    for (int it = 0; it <= 8000; ++it) {
        client->update(it);
        const std::vector<ViewDroppedItem>& drops =
                client->worldState().droppedItems;
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

// --- Ordeñe: tocar la vaca lista, animar, y recibir la leche -------------

TEST(MockWorldClientTest, MilkingAReadyCowYieldsMilkAfterTheWindow) {
    const std::unique_ptr<GameClient> client = makeClient(worldCatalog());

    // Avanzar hasta que alguna vaca quede lista para ordeñar.
    int it = 0;
    int cowId = -1;
    for (; it <= 8000; ++it) {
        client->update(it);
        for (const game::ViewEntity& entity : client->worldState().entities) {
            if (entity.indication == world::Indication::ReadyToMilk) {
                cowId = entity.id;
                break;
            }
        }
        if (cowId >= 0) {
            break;
        }
    }
    ASSERT_GE(cowId, 0);

    // Acercarse: la vaca camina, así que seguirla paso a paso hasta
    // estar a menos de una celda (la cercanía la valida la fuente).
    for (int steps = 0; steps < 200; ++steps) {
        const game::LocalWorldView& view = client->worldState();
        const game::ViewEntity& player = view.entities[0];
        const game::ViewEntity* cow = nullptr;
        for (const game::ViewEntity& entity : view.entities) {
            if (entity.id == cowId) {
                cow = &entity;
                break;
            }
        }
        ASSERT_NE(cow, nullptr);
        if (std::abs(cow->cellX - player.cellX) <= 1 &&
            std::abs(cow->cellY - player.cellY) <= 1) {
            break;
        }
        client->sendCommand(walk(toward(player.cellX, player.cellY,
                                        cow->cellX, cow->cellY)));
        client->update(++it);
    }

    // Tocar la vaca: arranca el ordeñe (el jugador queda con la acción
    // Milk, mirando a la vaca).
    client->sendCommand(take(world::Direction::Front));
    client->update(++it);
    const game::ViewEntity& milking = client->worldState().entities[0];
    EXPECT_EQ(milking.action, world::ActionKind::Milk);

    // Tras la ventana (2 ciclos de la animación): la leche entra al
    // inventario con su evento de obtenido, y la vaca deja de estar
    // lista (su diálogo se cierra).
    bool obtainedMilk = false;
    bool hasMilk = false;
    for (int n = 0; n < 200 && !hasMilk; ++n) {
        client->update(++it);
        for (const game::ViewObtained& event : client->worldState().obtained) {
            if (event.contentId == "milk") {
                obtainedMilk = true;
            }
        }
        for (const game::ViewInventorySlot& slot :
             client->worldState().inventory) {
            if (slot.contentId == "milk" && slot.count > 0) {
                hasMilk = true;
            }
        }
    }
    EXPECT_TRUE(obtainedMilk);
    EXPECT_TRUE(hasMilk);

    // El jugador vuelve a la normalidad y la vaca ya no está lista.
    const game::LocalWorldView& end = client->worldState();
    EXPECT_NE(end.entities[0].action, world::ActionKind::Milk);
    for (const game::ViewEntity& entity : end.entities) {
        if (entity.id == cowId) {
            EXPECT_FALSE(entity.indication.has_value());
        }
    }
}

// --- Recoger: tocar el huevo del piso, animar, y recibirlo ----------------

TEST(MockWorldClientTest, PickingUpAGroundEggYieldsItAfterTheWindow) {
    const std::unique_ptr<GameClient> client = makeClient(worldCatalog());

    // Avanzar hasta que un animal deje algo en el piso.
    int it = 0;
    for (; it <= 8000 && client->worldState().droppedItems.empty(); ++it) {
        client->update(it);
    }
    ASSERT_FALSE(client->worldState().droppedItems.empty());

    // Acercarse al ítem (está quieto: basta caminar hacia él).
    for (int steps = 0; steps < 200; ++steps) {
        const game::LocalWorldView& view = client->worldState();
        const game::ViewEntity& player = view.entities[0];
        const game::ViewDroppedItem& item = view.droppedItems[0];
        if (std::abs(item.cellX - player.cellX) <= 1 &&
            std::abs(item.cellY - player.cellY) <= 1) {
            break;
        }
        client->sendCommand(walk(toward(player.cellX, player.cellY,
                                        item.cellX, item.cellY)));
        client->update(++it);
    }

    const game::ViewDroppedItem& target = client->worldState().droppedItems[0];
    const std::string picked = target.contentId;
    const int pickedCellX = target.cellX;
    const int pickedCellY = target.cellY;

    // Tocar el ítem: arranca el recogido — el huevo queda en las manos
    // (el piso lo pierde) y el jugador hace la acción Pickup.
    client->sendCommand(take(world::Direction::Front));
    client->update(++it);
    const game::ViewEntity& picking = client->worldState().entities[0];
    EXPECT_EQ(picking.action, world::ActionKind::Pickup);
    ASSERT_TRUE(picking.activeTool.has_value());
    EXPECT_EQ(*picking.activeTool, world::ItemKind::Egg);
    for (const game::ViewDroppedItem& item :
         client->worldState().droppedItems) {
        EXPECT_FALSE(item.cellX == pickedCellX && item.cellY == pickedCellY)
                << "el ítem siguió en el piso durante el recogido";
    }

    // Tras la ventana (1 ciclo de la animación): el ítem entra al
    // inventario con su evento de obtenido.
    bool obtained = false;
    bool hasIt = false;
    for (int n = 0; n < 200 && !hasIt; ++n) {
        client->update(++it);
        for (const game::ViewObtained& event :
             client->worldState().obtained) {
            if (event.contentId == picked) {
                obtained = true;
            }
        }
        for (const game::ViewInventorySlot& slot :
             client->worldState().inventory) {
            if (slot.contentId == picked && slot.count > 0) {
                hasIt = true;
            }
        }
    }
    EXPECT_TRUE(obtained);
    EXPECT_TRUE(hasIt);

    // El jugador vuelve a la normalidad.
    EXPECT_NE(client->worldState().entities[0].action,
              world::ActionKind::Pickup);
}

}  // namespace
