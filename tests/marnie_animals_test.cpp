#include <memory>
#include <string>
#include <variant>

#include <gtest/gtest.h>

#include "../server/domain/community/community_set_rewards.h"
#include "../server/domain/map/portal.h"
#include "../server/domain/map/world.h"
#include "../server/domain/npc/marnie.h"
#include "../server/world_view.h"

namespace {

class AnimalShopScenario {
public:
    World world;
    Marnie marnie;

    AnimalShopScenario(): world(10, 10), marnie(0, 0) {
        world.add_player(1, std::make_unique<Player>(1, 5, 5, "town"));
        world.get_player(1).set_gold(5000);
    }

    std::string buy(const std::string& item, uint16_t quantity, uint8_t playerId = 1) {
        return marnie.process_command(world.get_player(playerId),
                                     "/comprar " + item + " " + std::to_string(quantity), world);
    }

    std::size_t count(const std::string& kind, const std::string& sceneId = "barn") {
        std::size_t total = 0;
        Scene& scene = world.get_scene(sceneId);
        for (uint16_t y = 0; y < scene.get_height(); ++y) {
            for (uint16_t x = 0; x < scene.get_width(); ++x) {
                for (const IEntity* entity: scene.get_entity_at(x, y)) {
                    const auto animal = entity->animal_snapshot();
                    if (animal && animal->kind == kind) ++total;
                }
            }
        }
        return total;
    }

    void block_barn() {
        Scene& barn = world.get_scene("barn");
        for (uint16_t y = 0; y < barn.get_height(); ++y) {
            for (uint16_t x = 0; x < barn.get_width(); ++x) {
                barn.set_wall(x, y, true);
            }
        }
    }
};

TEST(MarnieAnimalsTest, CompraCreaAnimalesSoloEnBarnYCobraElPrecio) {
    AnimalShopScenario shop;
    EXPECT_EQ(shop.count("cow"), 0u);
    EXPECT_EQ(shop.count("chicken"), 0u);
    EXPECT_NE(shop.buy("vacas", 2).find("¡Listo!"), std::string::npos);
    EXPECT_NE(shop.buy("gallinas", 3).find("¡Listo!"), std::string::npos);
    EXPECT_EQ(shop.world.get_player(1).get_gold(), 4780);
    EXPECT_EQ(shop.count("cow"), 2u);
    EXPECT_EQ(shop.count("chicken"), 3u);
    EXPECT_EQ(shop.count("cow", "farm"), 0u);
    EXPECT_EQ(shop.count("chicken", "farm"), 0u);

    // La sincronización solo envía los animales a quien está en el establo.
    WorldView view;
    for (const ServerMessage& message: view.sync(shop.world, 1)) {
        EXPECT_FALSE(std::holds_alternative<ServerAnimal>(message));
    }
    shop.world.transfer_player(1, "barn", 9, 9);
    std::size_t synced = 0;
    for (const ServerMessage& message: view.sync(shop.world, 1)) {
        if (std::holds_alternative<ServerAnimal>(message)) ++synced;
    }
    EXPECT_EQ(synced, 5u);
}

TEST(MarnieAnimalsTest, LimiteCincoVacasRechazaLaSextaSinCobrar) {
    AnimalShopScenario shop;
    shop.buy("vacas", 5);
    const uint16_t gold = shop.world.get_player(1).get_gold();
    EXPECT_NE(shop.buy("vacas", 1).find("máximo"), std::string::npos);
    EXPECT_EQ(shop.count("cow"), 5u);
    EXPECT_EQ(shop.world.get_player(1).get_gold(), gold);
}

TEST(MarnieAnimalsTest, LimiteDiezGallinasEsIndependienteDelDeVacas) {
    AnimalShopScenario shop;
    shop.buy("vacas", 5);
    shop.buy("gallinas", 10);
    const uint16_t gold = shop.world.get_player(1).get_gold();
    EXPECT_NE(shop.buy("gallinas", 1).find("máximo"), std::string::npos);
    EXPECT_EQ(shop.count("cow"), 5u);
    EXPECT_EQ(shop.count("chicken"), 10u);
    EXPECT_EQ(shop.world.get_player(1).get_gold(), gold);
}

TEST(MarnieAnimalsTest, CompraQueCruzaElTopeNoSeCompletaParcialmente) {
    AnimalShopScenario shop;
    shop.buy("vacas", 4);
    const uint16_t gold = shop.world.get_player(1).get_gold();
    shop.buy("vacas", 2);
    EXPECT_EQ(shop.count("cow"), 4u);
    EXPECT_EQ(shop.world.get_player(1).get_gold(), gold);
    shop.buy("vacas", 65535);
    EXPECT_EQ(shop.count("cow"), 4u);
    EXPECT_EQ(shop.world.get_player(1).get_gold(), gold);
}

TEST(MarnieAnimalsTest, SinOroNoCreaAnimales) {
    AnimalShopScenario shop;
    shop.world.get_player(1).set_gold(79);
    EXPECT_NE(shop.buy("vacas", 1).find("No te alcanza"), std::string::npos);
    EXPECT_EQ(shop.count("cow"), 0u);
    EXPECT_EQ(shop.world.get_player(1).get_gold(), 79);
}

TEST(MarnieAnimalsTest, SinEspacioParaTodoElLoteNoCobraNiCreaAnimales) {
    AnimalShopScenario shop;
    shop.block_barn();
    shop.world.get_scene("barn").set_wall(2, 2, false);
    EXPECT_NE(shop.buy("gallinas", 2).find("espacio"), std::string::npos);
    EXPECT_EQ(shop.count("chicken"), 0u);
    EXPECT_EQ(shop.world.get_player(1).get_gold(), 5000);
    shop.buy("gallinas", 1);
    EXPECT_EQ(shop.count("chicken"), 1u);
    EXPECT_EQ(shop.world.get_player(1).get_gold(), 4980);
}

TEST(MarnieAnimalsTest, SpawnEvitaParedesPortalesYJugadores) {
    AnimalShopScenario shop;
    Scene& barn = shop.world.get_scene("barn");
    barn.set_wall(0, 0, true);
    barn.set_entity_at(1, 0, std::make_unique<Portal>(1, 0, "farm", 1, 1));
    shop.world.add_player(2, std::make_unique<Player>(2, 2, 0, "barn"));
    shop.buy("vacas", 1);
    EXPECT_TRUE(barn.get_entity_at(0, 0).empty());
    EXPECT_EQ(barn.get_entity_at(1, 0).size(), 1u);
    EXPECT_TRUE(barn.get_entity_at(2, 0).empty());
    ASSERT_EQ(barn.get_entity_at(3, 0).size(), 1u);
    EXPECT_TRUE(barn.get_entity_at(3, 0)[0]->animal_snapshot().has_value());
}

TEST(MarnieAnimalsTest, LosTopesSonCompartidosEntreJugadores) {
    AnimalShopScenario shop;
    shop.world.add_player(2, std::make_unique<Player>(2, 5, 6, "town"));
    shop.world.get_player(2).set_gold(1000);
    shop.buy("vacas", 3);
    shop.buy("vacas", 3, 2);
    EXPECT_EQ(shop.count("cow"), 3u);
    EXPECT_EQ(shop.world.get_player(2).get_gold(), 1000);
    shop.buy("vacas", 2, 2);
    EXPECT_EQ(shop.count("cow"), 5u);
    EXPECT_EQ(shop.world.get_player(2).get_gold(), 840);
}

TEST(MarnieAnimalsTest, RecompensaComunitariaEntregaUnaVacaYDosGallinasEnBarn) {
    AnimalShopScenario shop;
    CommunitySetRewards().apply(shop.world, CommunitySet::ANIMALS);
    EXPECT_EQ(shop.count("cow"), 1u);
    EXPECT_EQ(shop.count("chicken"), 2u);
    EXPECT_EQ(shop.count("cow", "farm"), 0u);
    EXPECT_EQ(shop.count("chicken", "farm"), 0u);
    EXPECT_EQ(shop.world.get_player(1).get_gold(), 5000);
}

TEST(MarnieAnimalsTest, MundoSinEstabloRechazaCompraSinCobrar) {
    worldmodel::World model;
    worldmodel::Map farm;
    farm.id = "farm";
    farm.kind = worldmodel::MapKind::Farm;
    farm.width = 5;
    farm.height = 5;
    model.maps.emplace("farm", std::move(farm));
    World world(model);
    Player player(1, 1, 1, "farm");
    player.set_gold(1000);
    Marnie marnie(0, 0);
    EXPECT_NE(marnie.process_command(player, "/comprar vacas 1", world).find("establo disponible"),
              std::string::npos);
    EXPECT_EQ(player.get_gold(), 1000);
}

TEST(MarnieAnimalsTest, MundoDelEditorUsaElEstabloYRespetaSusColisiones) {
    worldmodel::World model;
    worldmodel::Map farm;
    farm.id = "farm";
    farm.kind = worldmodel::MapKind::Farm;
    farm.width = 4;
    farm.height = 4;
    model.maps.emplace("farm", farm);
    worldmodel::Map barn = farm;
    barn.id = "barn";
    barn.kind = worldmodel::MapKind::Barn;
    barn.collision.assign(16, false);
    barn.collision[0] = true;
    model.maps.emplace("barn", barn);
    World world(model);
    Player player(1, 1, 1, "farm");
    player.set_gold(1000);
    Marnie marnie(0, 0);

    EXPECT_NE(marnie.process_command(player, "/comprar gallinas 1", world).find("¡Listo!"),
              std::string::npos);
    EXPECT_TRUE(world.get_scene("barn").get_entity_at(0, 0).empty());
    ASSERT_EQ(world.get_scene("barn").get_entity_at(1, 0).size(), 1u);
    EXPECT_TRUE(world.get_scene("farm").get_entity_at(1, 0).empty());
    world.tick_day();
    EXPECT_EQ(world.get_scene("barn").get_entity_at(1, 0).size(), 2u);
    EXPECT_EQ(player.get_gold(), 980);
}

TEST(MarnieAnimalsTest, ElDescuentoSeAplicaAntesDelCobro) {
    AnimalShopScenario shop;
    shop.world.activate_market_rewards();
    shop.buy("vacas", 1);
    shop.buy("gallinas", 1);
    EXPECT_EQ(shop.world.get_player(1).get_gold(), 4950);
    EXPECT_EQ(shop.count("cow"), 1u);
    EXPECT_EQ(shop.count("chicken"), 1u);
}

TEST(MarnieAnimalsTest, RecompensaConTopesLlenosPagaElValorCompleto) {
    AnimalShopScenario shop;
    shop.buy("vacas", 5);
    shop.buy("gallinas", 10);
    const uint16_t gold = shop.world.get_player(1).get_gold();
    CommunitySetRewards().apply(shop.world, CommunitySet::ANIMALS);
    EXPECT_EQ(shop.count("cow"), 5u);
    EXPECT_EQ(shop.count("chicken"), 10u);
    EXPECT_EQ(shop.world.get_player(1).get_gold(), gold + 120);
}

TEST(MarnieAnimalsTest, RecompensaParcialPagaSoloLosAnimalesNoEntregados) {
    AnimalShopScenario shop;
    shop.buy("vacas", 4);
    shop.buy("gallinas", 9);
    const uint16_t gold = shop.world.get_player(1).get_gold();
    CommunitySetRewards().apply(shop.world, CommunitySet::ANIMALS);
    EXPECT_EQ(shop.count("cow"), 5u);
    EXPECT_EQ(shop.count("chicken"), 10u);
    EXPECT_EQ(shop.world.get_player(1).get_gold(), gold + 20);
}

TEST(MarnieAnimalsTest, RecompensaSinCeldasLibresPagaOroAunqueNoEsteAlTope) {
    AnimalShopScenario shop;
    shop.block_barn();
    CommunitySetRewards().apply(shop.world, CommunitySet::ANIMALS);
    EXPECT_EQ(shop.count("cow"), 0u);
    EXPECT_EQ(shop.count("chicken"), 0u);
    EXPECT_EQ(shop.world.get_player(1).get_gold(), 5120);
}

TEST(MarnieAnimalsTest, RecompensaConUnaCeldaEntregaLaVacaYCompensaLasGallinas) {
    AnimalShopScenario shop;
    shop.block_barn();
    shop.world.get_scene("barn").set_wall(2, 2, false);
    CommunitySetRewards().apply(shop.world, CommunitySet::ANIMALS);
    EXPECT_EQ(shop.count("cow"), 1u);
    EXPECT_EQ(shop.count("chicken"), 0u);
    EXPECT_EQ(shop.world.get_player(1).get_gold(), 5040);
}

TEST(MarnieAnimalsTest, MovimientoPermaneceEnBarnYPreservaLaEscenaActiva) {
    AnimalShopScenario shop;
    shop.buy("gallinas", 10);
    shop.world.set_active_scene("farm");
    for (int tick = 0; tick < 50; ++tick) {
        shop.world.tick_animals();
    }
    EXPECT_EQ(shop.world.get_active_scene(), "farm");
    EXPECT_EQ(shop.count("chicken"), 10u);
    EXPECT_EQ(shop.count("chicken", "farm"), 0u);
}

}  // namespace
