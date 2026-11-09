#include "world_view.h"

#include <memory>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "domain/animals/chicken.h"
#include "domain/entities/player.h"
#include "domain/map/world.h"
#include "domain/minerals/coal_rock.h"
#include "domain/minerals/gold_rock.h"

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

TEST(WorldViewTest, EverySyncStartsWithAResetAndDescribesTheWholeState) {
    World world(5, 5);
    populate(world);
    WorldView view;

    const std::vector<ServerMessage> messages = view.sync(world, 1);

    ASSERT_FALSE(messages.empty());
    EXPECT_TRUE(std::holds_alternative<ServerReset>(messages.front()));
    EXPECT_EQ(only<ServerStatus>(messages).size(), 1u);
    const auto resources = only<ServerResource>(messages);
    ASSERT_EQ(resources.size(), 1u);
    EXPECT_EQ(resources[0].x, 2);
    EXPECT_EQ(resources[0].y, 1);
    EXPECT_EQ(resources[0].content_id, "coal");
    EXPECT_EQ(resources[0].damage, ResourceDamage::Intact);
    // Solo los slots ocupados: el reset ya borró los vacíos del cliente.
    EXPECT_EQ(only<ServerInventorySlot>(messages).size(), 4u);
    // El inventario inicial no es una obtención: eso lo emite Match.
    EXPECT_TRUE(only<ServerObtained>(messages).empty());
    // Cierra el lote: el cliente aplica todo recién al ver este mensaje
    // (sin el cierre podría dibujar un frame sin entidades = parpadeo).
    ASSERT_GT(messages.size(), 1u);
    EXPECT_TRUE(std::holds_alternative<ServerSyncEnd>(messages.back()));

    // Sin memoria entre llamadas: sin cambios, el mismo estado sale otra vez.
    const std::vector<ServerMessage> again = view.sync(world, 1);
    EXPECT_EQ(again.size(), messages.size());
    EXPECT_TRUE(std::holds_alternative<ServerReset>(again.front()));
    EXPECT_TRUE(std::holds_alternative<ServerSyncEnd>(again.back()));
}

TEST(WorldViewTest, StatusCarriesEachPlayersAppearance) {
    World world(5, 5);
    populate(world);
    world.add_player(2, std::make_unique<Player>(2, 4, 4));
    CharacterAppearance look;
    look.skin = 2;
    look.hair = 5;
    look.accessory = 7;
    world.set_player_appearance(2, look);
    WorldView view;

    // Cada joiner recibe la apariencia de todos en el mismo lote.
    const auto statuses = only<ServerStatus>(view.sync(world, 1));
    ASSERT_EQ(statuses.size(), 2u);
    for (const ServerStatus& status: statuses) {
        if (status.player_id == 2) {
            EXPECT_EQ(status.appearance.skin, 2);
            EXPECT_EQ(status.appearance.hair, 5);
            EXPECT_EQ(status.appearance.accessory, 7);
        } else {
            EXPECT_EQ(status.appearance.skin, 0);
        }
    }
}

TEST(WorldViewTest, BlockedMoveKeepsSendingTheSameState) {
    World world(5, 5);
    populate(world);
    WorldView view;

    // La roca bloquea el paso y el jugador ya mira hacia ella.
    world.move_player(1, Direction::RIGHT);

    const auto statuses = only<ServerStatus>(view.sync(world, 1));
    ASSERT_EQ(statuses.size(), 1u);
    EXPECT_EQ(statuses[0].x, 1);
    EXPECT_EQ(statuses[0].y, 1);
}

TEST(WorldViewTest, MiningRemovesRockAndShowsTheDropInItsPlace) {
    World world(5, 5);
    populate(world);
    WorldView view;

    world.use_tool(1);
    const std::vector<ServerMessage> messages = view.sync(world, 1);

    // La roca no vuelve en el lote: el reset la borra del cliente.
    EXPECT_TRUE(only<ServerResource>(messages).empty());
    const auto dropped = only<ServerDropped>(messages);
    ASSERT_EQ(dropped.size(), 1u);
    EXPECT_EQ(dropped[0].content_id, "coal");
    EXPECT_EQ(dropped[0].x, 2);
    EXPECT_EQ(dropped[0].y, 1);
    // El orbe de mineral no se levanta con Take: el cliente no lo anima.
    EXPECT_FALSE(dropped[0].interactive);
}

TEST(WorldViewTest, WalkingOntoTheDropFreesTheCellAndFillsTheSlot) {
    World world(5, 5);
    populate(world);
    world.add_player(2, std::make_unique<Player>(2, 4, 4));
    WorldView view;

    world.use_tool(1);
    world.move_player(1, Direction::RIGHT);

    // El inventario es propio de cada jugador: el carbón sale solo en el sync
    // de quien lo levantó.
    const auto own = only<ServerInventorySlot>(view.sync(world, 1));
    ASSERT_EQ(own.size(), 5u);
    EXPECT_EQ(own[4].slot, 4);
    EXPECT_EQ(own[4].count, 1);
    EXPECT_EQ(own[4].content_id, "coal");
    EXPECT_EQ(only<ServerInventorySlot>(view.sync(world, 2)).size(), 4u);
}

TEST(WorldViewTest, StackedPickupRaisesTheSlotCount) {
    World world(5, 5);
    populate(world);
    world.set_entity_at(3, 1, std::make_unique<CoalRock>(3, 1));
    WorldView view;

    world.use_tool(1);
    world.move_player(1, Direction::RIGHT);
    world.use_tool(1);
    world.move_player(1, Direction::RIGHT);

    const auto slots = only<ServerInventorySlot>(view.sync(world, 1));
    ASSERT_EQ(slots.size(), 5u);
    EXPECT_EQ(slots[4].slot, 4);
    EXPECT_EQ(slots[4].count, 2);
}

TEST(WorldViewTest, HitsAdvanceTheDamageInTheSentState) {
    World world(4, 4);
    world.add_player(1, std::make_unique<Player>(1, 0, 1));
    world.set_player_facing(1, Direction::RIGHT);
    world.set_entity_at(1, 1, std::make_unique<GoldRock>(1, 1));
    WorldView view;

    // La roca de oro aguanta 4 golpes: el estado la muestra con el daño actual.
    world.use_tool(1);
    const auto resources = only<ServerResource>(view.sync(world, 1));
    ASSERT_EQ(resources.size(), 1u);
    EXPECT_EQ(resources[0].damage, ResourceDamage::Cracked);

    world.use_tool(1);
    world.use_tool(1);
    const auto battered = only<ServerResource>(view.sync(world, 1));
    ASSERT_EQ(battered.size(), 1u);
    EXPECT_EQ(battered[0].damage, ResourceDamage::Broken);

    // Al cuarto golpe deja de ser un recurso: la celda pasa a ser el drop.
    world.use_tool(1);
    EXPECT_TRUE(only<ServerResource>(view.sync(world, 1)).empty());
}

TEST(WorldViewTest, DroppedEggIsSentInteractive) {
    World world(5, 5);
    world.add_player(1, std::make_unique<Player>(1, 1, 1));
    world.add_animal(std::make_unique<Chicken>(3, 3));
    WorldView view;

    world.tick_day();  // la gallina pone un huevo en su celda

    const auto dropped = only<ServerDropped>(view.sync(world, 1));
    ASSERT_EQ(dropped.size(), 1u);
    EXPECT_EQ(dropped[0].content_id, "egg");
    EXPECT_EQ(dropped[0].x, 3);
    EXPECT_EQ(dropped[0].y, 3);
    // El huevo sí se levanta con Take: el cliente con eso anima la recogida.
    EXPECT_TRUE(dropped[0].interactive);
}

TEST(WorldViewTest, TheEggWaitsForTakeInsteadOfPickingItselfUp) {
    World world(5, 5);
    world.add_player(1, std::make_unique<Player>(1, 1, 1));
    world.add_animal(std::make_unique<Chicken>(2, 1));
    WorldView view;
    world.tick_day();

    world.move_player(1, Direction::RIGHT);

    // Sigue en la celda, junto a la gallina: caminar no lo levanta.
    EXPECT_EQ(world.get_entity_at(2, 1).size(), 2u);
    EXPECT_EQ(only<ServerInventorySlot>(view.sync(world, 1)).size(), 4u);

    world.take_item(1);

    // El Take de la celda propia sí lo levanta y sale del estado.
    const auto own = only<ServerInventorySlot>(view.sync(world, 1));
    ASSERT_EQ(own.size(), 5u);
    EXPECT_EQ(own[4].slot, 4);
    EXPECT_EQ(own[4].content_id, "egg");
    EXPECT_TRUE(only<ServerDropped>(view.sync(world, 1)).empty());
}

TEST(WorldViewTest, TakeReachesAnEggInAnAdjacentCellButNotFartherOne) {
    World world(5, 5);
    world.add_player(1, std::make_unique<Player>(1, 1, 1));
    world.add_animal(std::make_unique<Chicken>(3, 1));
    WorldView view;
    world.tick_day();

    // El huevo está a dos celdas: el Take no lo alcanza.
    world.take_item(1);
    EXPECT_EQ(only<ServerInventorySlot>(view.sync(world, 1)).size(), 4u);

    // Desde la celda de al lado sí.
    world.move_player(1, Direction::RIGHT);
    world.take_item(1);
    EXPECT_EQ(only<ServerInventorySlot>(view.sync(world, 1)).size(), 5u);
    EXPECT_TRUE(only<ServerDropped>(view.sync(world, 1)).empty());
}

}  // namespace
