#include "world_view.h"

#include <memory>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "domain/animals/chicken.h"
#include "domain/crops/potato.h"
#include "domain/enemies/orc.h"
#include "domain/enemies/skeleton.h"
#include "domain/enemies/skull.h"
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
    EXPECT_EQ(only<ServerInventorySlot>(messages).size(), 8u);
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
    ASSERT_EQ(own.size(), 9u);
    EXPECT_EQ(own[8].slot, 8);
    EXPECT_EQ(own[8].count, 1);
    EXPECT_EQ(own[8].content_id, "coal");
    // tradable lo pone el dominio: el pico (slot 0) no se regala y el
    // carbón recogido sí, que es la regla que lee el cliente.
    EXPECT_FALSE(own[0].tradable);
    EXPECT_TRUE(own[8].tradable);
    EXPECT_EQ(only<ServerInventorySlot>(view.sync(world, 2)).size(), 8u);
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
    ASSERT_EQ(slots.size(), 9u);
    EXPECT_EQ(slots[8].slot, 8);
    EXPECT_EQ(slots[8].count, 2);
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
    EXPECT_EQ(only<ServerInventorySlot>(view.sync(world, 1)).size(), 8u);

    world.take_item(1);

    // El Take de la celda propia sí lo levanta y sale del estado.
    const auto own = only<ServerInventorySlot>(view.sync(world, 1));
    ASSERT_EQ(own.size(), 9u);
    EXPECT_EQ(own[8].slot, 8);
    EXPECT_EQ(own[8].content_id, "egg");
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
    EXPECT_EQ(only<ServerInventorySlot>(view.sync(world, 1)).size(), 8u);

    // Desde la celda de al lado sí.
    world.move_player(1, Direction::RIGHT);
    world.take_item(1);
    EXPECT_EQ(only<ServerInventorySlot>(view.sync(world, 1)).size(), 9u);
    EXPECT_TRUE(only<ServerDropped>(view.sync(world, 1)).empty());
}

// Jugador 1 en (1,1) mirando a la derecha con la semilla de frutilla en el slot 4.
Player* populateWithStrawberrySeed(World& world) {
    auto player = std::make_unique<Player>(1, 1, 1);
    Player* ref = player.get();
    world.add_player(1, std::move(player));
    world.set_player_facing(1, Direction::RIGHT);
    world.get_cell(2, 1).till_soil();
    ref->select_slot(4);
    return ref;
}

TEST(WorldViewTest, PlantedSeedIsSentAsAGrowingCropThatAsksForWater) {
    World world(5, 5);
    populateWithStrawberrySeed(world);
    WorldView view;

    world.use_tool(1);

    const auto crops = only<ServerCrop>(view.sync(world, 1));
    ASSERT_EQ(crops.size(), 1u);
    EXPECT_EQ(crops[0].x, 2);
    EXPECT_EQ(crops[0].y, 1);
    EXPECT_EQ(crops[0].crop_id, "strawberry");
    EXPECT_EQ(crops[0].stage, 0);
    EXPECT_TRUE(crops[0].needs_water);
    EXPECT_FALSE(crops[0].interactive);
    // La semilla se gasta y la tierra arada sale como efecto del suelo.
    EXPECT_EQ(only<ServerInventorySlot>(view.sync(world, 1)).size(), 7u);
    const auto ground = only<ServerGroundEffect>(view.sync(world, 1));
    ASSERT_EQ(ground.size(), 1u);
    EXPECT_EQ(ground[0].effect, GroundEffect::Dug);
}

TEST(WorldViewTest, CropGrowsDayByDayAndIsReadyToHarvestAfterItsDays) {
    World world(5, 5);
    populateWithStrawberrySeed(world);
    WorldView view;
    world.use_tool(1);

    // La frutilla tarda 4 días: a mitad de camino ya creció.
    world.process_end_of_day_floor();
    const auto growing = only<ServerCrop>(view.sync(world, 1));
    ASSERT_EQ(growing.size(), 1u);
    EXPECT_EQ(growing[0].stage, 1);
    EXPECT_FALSE(growing[0].interactive);

    for (int day = 0; day < 3; ++day) {
        world.process_end_of_day_floor();
    }
    const auto ready = only<ServerCrop>(view.sync(world, 1));
    ASSERT_EQ(ready.size(), 1u);
    EXPECT_EQ(ready[0].stage, 6);
    EXPECT_TRUE(ready[0].interactive);
    EXPECT_FALSE(ready[0].needs_water);
}

TEST(WorldViewTest, TakeHarvestsAReadyCropInAnAdjacentCell) {
    World world(5, 5);
    world.add_player(1, std::make_unique<Player>(1, 1, 1));
    world.set_entity_at(2, 1, std::make_unique<Potato>(2, 1));
    WorldView view;
    for (int day = 0; day < 3; ++day) {
        world.process_end_of_day_floor();  // la papa tarda 3 días
    }

    world.take_item(1);

    // La cosecha sale en el banner (pickup) y en el inventario, y el cultivo desaparece.
    const auto taken = world.take_pickups();
    ASSERT_EQ(taken.size(), 1u);
    EXPECT_EQ(taken[0].item, ItemID::POTATO);
    EXPECT_TRUE(only<ServerCrop>(view.sync(world, 1)).empty());
    EXPECT_EQ(only<ServerInventorySlot>(view.sync(world, 1)).size(), 9u);
}

TEST(WorldViewTest, WateredSoilIsSentAsDugThenWatered) {
    World world(5, 5);
    populate(world);
    world.get_cell(3, 3).till_soil();
    world.get_cell(3, 3).water_soil();
    WorldView view;

    const auto ground = only<ServerGroundEffect>(view.sync(world, 1));
    ASSERT_EQ(ground.size(), 2u);
    EXPECT_EQ(ground[0].effect, GroundEffect::Dug);
    EXPECT_EQ(ground[1].effect, GroundEffect::Watered);
}

TEST(WorldViewTest, LosCharcosDeSangreSeTransmitenEnCadaLote) {
    World world(5, 5);
    populate(world);
    world.damage_player(1, 85);  // 100 -> 15: por debajo del 20 %, deja sangre
    WorldView view;

    // Sin tierra tratada, el unico GroundEffect del lote es el charco:
    // va en el mismo transporte que la tierra, para que TODOS los
    // jugadores sepan donde pueden revivir los vampiros.
    const auto ground = only<ServerGroundEffect>(view.sync(world, 1));
    ASSERT_EQ(ground.size(), 1u);
    EXPECT_EQ(ground[0].x, 1);
    EXPECT_EQ(ground[0].y, 1);
    EXPECT_EQ(ground[0].effect, GroundEffect::Blood);
}

TEST(WorldViewTest, CropsAndSeedsUseTheSharedContentIds) {
    EXPECT_EQ(content_id_for(ItemID::POTATO), "potato");
    EXPECT_EQ(content_id_for(ItemID::GREEN_ONION), "onion");
    EXPECT_EQ(content_id_for(ItemID::PUMPKIN_SEED), "pumpkin_seeds");
}

TEST(WorldViewTest, UnEnemigoVivoSeEnviaConSuIdentidadVidaYPosicion) {
    World world(5, 5);
    populate(world);
    world.add_enemy(std::make_unique<Skeleton>(3, 3));
    WorldView view;

    const auto enemies = only<ServerEnemy>(view.sync(world, 1));
    ASSERT_EQ(enemies.size(), 1u);
    EXPECT_EQ(enemies[0].id, 1000);
    EXPECT_EQ(enemies[0].kind, "skeleton");
    EXPECT_EQ(enemies[0].x, 3);
    EXPECT_EQ(enemies[0].y, 3);
    EXPECT_EQ(enemies[0].health, 10);
    EXPECT_EQ(enemies[0].action, EnemyAction::Idle);
}

TEST(WorldViewTest, UnGolpeMandaDanoYLaMuerteSigueVisibleConVidaCero) {
    World world(5, 5);
    populate(world);
    auto skeleton = std::make_unique<Skeleton>(3, 3);
    Skeleton* ptr = skeleton.get();
    world.add_enemy(std::move(skeleton));
    WorldView view;

    ptr->take_damage(4);
    const auto hurt = only<ServerEnemy>(view.sync(world, 1));
    ASSERT_EQ(hurt.size(), 1u);
    EXPECT_EQ(hurt[0].health, 6);
    EXPECT_EQ(hurt[0].action, EnemyAction::Damage);

    ptr->take_damage(6);
    const auto dead = only<ServerEnemy>(view.sync(world, 1));
    ASSERT_EQ(dead.size(), 1u);
    EXPECT_EQ(dead[0].health, 0);
}

TEST(WorldViewTest, UnEnemigoQuePegaEnviaAtaque) {
    World world(5, 5);
    populate(world);
    world.add_enemy(std::make_unique<Skeleton>(2, 2));  // diagonal al jugador (1,1)
    WorldView view;

    world.tick_enemies_pursuit();
    const auto enemies = only<ServerEnemy>(view.sync(world, 1));
    ASSERT_EQ(enemies.size(), 1u);
    EXPECT_EQ(enemies[0].action, EnemyAction::Attack);
}

TEST(WorldViewTest, CadaEnemigoTieneSuPropioId) {
    World world(5, 5);
    populate(world);
    world.add_enemy(std::make_unique<Skeleton>(3, 3));
    world.add_enemy(std::make_unique<Orc>(4, 4));
    WorldView view;

    const auto enemies = only<ServerEnemy>(view.sync(world, 1));
    ASSERT_EQ(enemies.size(), 2u);
    EXPECT_NE(enemies[0].id, enemies[1].id);
    EXPECT_EQ(enemies[1].kind, "orc");
}

TEST(WorldViewTest, SinEnemigosNoSeEnviaNinguno) {
    World world(5, 5);
    populate(world);
    WorldView view;

    EXPECT_TRUE(only<ServerEnemy>(view.sync(world, 1)).empty());
}

TEST(WorldViewTest, StatusCarriesFaintedFlag) {
    World world(5, 5);
    populate(world);
    WorldView view;
    EXPECT_EQ(only<ServerStatus>(view.sync(world, 1))[0].fainted, 0u);
    world.faint_player(1);
    EXPECT_EQ(only<ServerStatus>(view.sync(world, 1))[0].fainted, 1u);
}

}  // namespace

TEST(WorldViewTest, LaLlamaSeEnviaComoProyectil) {
    World world(5, 5);
    populate(world);
    world.add_enemy(std::make_unique<Skull>(2, 2));  // a 1 de diagonal del jugador (1,1)
    WorldView view;

    world.tick_enemies();
    const auto projectiles = only<ServerProjectile>(view.sync(world, 1));
    ASSERT_EQ(projectiles.size(), 1u);
    EXPECT_EQ(projectiles[0].kind, "skull_fire");
    EXPECT_EQ(projectiles[0].x, 2);
    EXPECT_EQ(projectiles[0].y, 2);
    EXPECT_EQ(projectiles[0].direction, Direction::LEFT);
}
