#include <memory>
#include <variant>
#include <vector>

#include "common/common_command.h"
#include "game/AnimalOverlayClient.h"
#include "game/GameClient.h"
#include "game/LocalWorldView.h"
#include "game/MockGameClient.h"
#include "gtest/gtest.h"
#include "world/CharacterCatalogLoader.h"
#include "world/CreatureCatalog.h"
#include "world/ViewDirection.h"

namespace {

using game::AnimalOverlayClient;
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
    return std::make_unique<AnimalOverlayClient>(
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

TEST(AnimalOverlayClientTest, SpawnsAnimalsWhenTheCatalogHasThem) {
    const std::unique_ptr<GameClient> client = makeClient(worldCatalog());
    const LocalWorldView& view = client->worldState();
    // 3 gallinas + 3 vacas + jugador, en orden de spawn.
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

TEST(AnimalOverlayClientTest, WithoutAppearancesOnlyThePlayerShows) {
    const world::CharacterCatalog empty;
    const std::unique_ptr<GameClient> client = makeClient(empty);
    const LocalWorldView& view = client->worldState();
    ASSERT_EQ(view.entities.size(), 1u);
    EXPECT_EQ(view.entities[0].kind, CreatureKind::Player);
}

TEST(AnimalOverlayClientTest, AnimalIdsAreUnique) {
    const std::unique_ptr<GameClient> client = makeClient(worldCatalog());
    const LocalWorldView& view = client->worldState();
    for (std::size_t a = 0; a < view.entities.size(); ++a) {
        for (std::size_t b = a + 1; b < view.entities.size(); ++b) {
            EXPECT_NE(view.entities[a].id, view.entities[b].id);
        }
    }
}

TEST(AnimalOverlayClientTest, AnimalsMoveOverUpdateTicks) {
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

TEST(AnimalOverlayClientTest, AnimalActionMatchesItsMovement) {
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

TEST(AnimalOverlayClientTest, CommandsReachTheInnerSource) {
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

TEST(AnimalOverlayClientTest, ProducingAnimalsDropItemsOnTheGround) {
    const std::unique_ptr<GameClient> client = makeClient(worldCatalog());

    // El período de postura es largo (gama en el .cpp de la
    // simulación) pero acotado: con esta seed la primera postura cae
    // bien dentro de la caminata.
    const std::vector<ViewDroppedItem> drops = groundItemsAfter(*client, 3000);
    ASSERT_FALSE(drops.empty());

    for (const ViewDroppedItem& drop : drops) {
        // El ítem es uno de los que la tabla de postura declara para
        // algún kind (gallina -> huevo, vaca -> leche).
        const bool knownDrop =
                drop.contentId == "egg" || drop.contentId == "milk";
        EXPECT_TRUE(knownDrop);
        EXPECT_GE(drop.cellX, 0);
        EXPECT_LT(drop.cellX, kRoomColumns);
        EXPECT_GE(drop.cellY, 0);
        EXPECT_LT(drop.cellY, kRoomRows);
        // Nadie suelta un ítem en la celda del jugador.
        EXPECT_FALSE(drop.cellX == kPlayerX && drop.cellY == kPlayerY);
    }
}

TEST(AnimalOverlayClientTest, GroundItemCountNeverExceedsTheCap) {
    // Sin recogida, el tope de ítems en el suelo es lo que evita que
    // la vista se llene de ítems (regla silenciosa del "server
    // falso", como el límite de slots del inventario).
    const std::unique_ptr<GameClient> client = makeClient(worldCatalog());
    const std::vector<ViewDroppedItem> drops = groundItemsAfter(*client, 8000);
    EXPECT_LE(drops.size(), 3u);
}

TEST(AnimalOverlayClientTest, GroundItemsAreStableBetweenFrames) {
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

TEST(AnimalOverlayClientTest, GroundItemsNeverShareACell) {
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

}  // namespace
