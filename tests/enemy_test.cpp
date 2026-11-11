#include <memory>

#include <gtest/gtest.h>

#include "../server/domain/enemies/orc.h"
#include "../server/domain/enemies/skeleton.h"
#include "../server/domain/entities/player.h"
#include "../server/domain/map/world.h"

// ---------------------------------------------------------
// IAttackable
// ---------------------------------------------------------
TEST(EnemyTest, TakeDamageBajaLaVidaYDevuelveTrueSoloAlMatar) {
    Skeleton skeleton(1, 1);

    EXPECT_EQ(skeleton.get_health(), 10);
    EXPECT_FALSE(skeleton.take_damage(4));
    EXPECT_EQ(skeleton.get_health(), 6);

    EXPECT_TRUE(skeleton.take_damage(6));  // clamp a 0
    EXPECT_EQ(skeleton.get_health(), 0);

    EXPECT_FALSE(skeleton.take_damage(3));  // un muerto no recibe daño
    EXPECT_EQ(skeleton.get_health(), 0);
}

TEST(EnemyTest, SoloLaEspadaHaceDanioYMata) {
    World world(5, 5);
    Player attacker(1, 0, 0);
    auto owned = std::make_unique<Skeleton>(2, 2);
    Skeleton* skeleton = owned.get();
    world.add_enemy(std::move(owned));

    EXPECT_FALSE(world.can_walk_to(2, 2));  // vivo: bloquea el paso

    skeleton->interact(attacker, ToolType::PICKAXE, world);
    skeleton->interact(attacker, ToolType::HAND, world);
    EXPECT_EQ(skeleton->get_health(), 10);  // otras herramientas no hacen nada

    for (int hit = 0; hit < 10; ++hit) {  // la espada hace 1 de daño
        skeleton->interact(attacker, ToolType::SWORD, world);
    }
    EXPECT_EQ(skeleton->get_health(), 0);
    EXPECT_TRUE(world.can_walk_to(2, 2));  // cadáver: no bloquea

    skeleton->interact(attacker, ToolType::SWORD, world);  // golpe extra inútil
    EXPECT_EQ(skeleton->get_health(), 0);
}

TEST(EnemyTest, ElJugadorRecibeDanioYPisaEnCero) {
    Player player(1, 0, 0);

    EXPECT_EQ(player.get_health(), 100);
    EXPECT_FALSE(player.take_damage(60));
    EXPECT_EQ(player.get_health(), 40);
    EXPECT_TRUE(player.take_damage(40));
    EXPECT_EQ(player.get_health(), 0);
    EXPECT_FALSE(player.take_damage(3));  // ya sin vida
    EXPECT_EQ(player.get_health(), 0);
}

// ---------------------------------------------------------
// Revive del esqueleto (estado muerto en la celda)
// ---------------------------------------------------------
TEST(EnemyTest, ElEsqueletoReviveConVidaLlenaDespuesDeLosTicks) {
    World world(5, 5);
    Player attacker(1, 0, 0);
    auto owned = std::make_unique<Skeleton>(2, 2);
    Skeleton* skeleton = owned.get();
    world.add_enemy(std::move(owned));

    for (int hit = 0; hit < 10; ++hit) {
        skeleton->interact(attacker, ToolType::SWORD, world);
    }
    ASSERT_EQ(skeleton->get_health(), 0);

    for (int tick = 0; tick < 14; ++tick) {  // 14 de 15: sigue muerto
        world.tick_enemies();
    }
    EXPECT_EQ(skeleton->get_health(), 0);
    EXPECT_TRUE(world.can_walk_to(2, 2));

    world.tick_enemies();  // 15º tick: revive
    EXPECT_EQ(skeleton->get_health(), 10);
    EXPECT_FALSE(world.can_walk_to(2, 2));  // vuelve a bloquear
}

// ---------------------------------------------------------
// IMovable + comportamiento del esqueleto
// ---------------------------------------------------------
TEST(EnemyTest, ElEsqueletoAvanzaUnaCasillaYNoSaleDelMapa) {
    World world(5, 5);
    auto owned = std::make_unique<Skeleton>(1, 1);
    Skeleton* skeleton = owned.get();
    world.add_enemy(std::move(owned));

    EXPECT_TRUE(skeleton->move(world, Direction::RIGHT));
    EXPECT_EQ(skeleton->get_x(), 2);
    EXPECT_EQ(skeleton->get_y(), 1);

    EXPECT_TRUE(skeleton->move(world, Direction::UP));
    EXPECT_EQ(skeleton->get_y(), 0);

    EXPECT_FALSE(skeleton->move(world, Direction::UP));  // borde del mapa
    EXPECT_EQ(skeleton->get_x(), 2);
    EXPECT_EQ(skeleton->get_y(), 0);
}

TEST(EnemyTest, ElEsqueletoPersigueAlJugadorDentroDelRango) {
    World world(10, 10);
    world.add_player(1, std::make_unique<Player>(1, 5, 1));
    auto owned = std::make_unique<Skeleton>(1, 1);
    const Skeleton* skeleton = owned.get();
    world.add_enemy(std::move(owned));

    world.tick_enemies();

    EXPECT_EQ(skeleton->get_x(), 2);  // un paso hacia el jugador
    EXPECT_EQ(skeleton->get_y(), 1);
    EXPECT_EQ(world.get_entity_at(2, 1).size(), 1u);
    EXPECT_EQ(world.get_entity_at(1, 1).size(), 0u);
}

TEST(EnemyTest, ElEsqueletoGolpeaAlJugadorAdyacente) {
    World world(10, 10);
    world.add_player(1, std::make_unique<Player>(1, 2, 1));
    auto owned = std::make_unique<Skeleton>(1, 1);
    world.add_enemy(std::move(owned));

    world.tick_enemies();

    EXPECT_EQ(world.get_player(1).get_health(), 95);  // 5 de daño cuerpo a cuerpo
}

TEST(EnemyTest, ElEsqueletoIgnoraAlJugadorFueraDeRango) {
    World world(10, 10);
    world.add_player(1, std::make_unique<Player>(1, 9, 9));
    auto owned = std::make_unique<Skeleton>(0, 0);
    const Skeleton* skeleton = owned.get();
    world.add_enemy(std::move(owned));

    world.tick_enemies();

    EXPECT_EQ(skeleton->get_x(), 0);  // fuera de rango (distancia 9 > 5)
    EXPECT_EQ(skeleton->get_y(), 0);
    EXPECT_EQ(world.get_player(1).get_health(), 100);
}

// ---------------------------------------------------------
// Orc
// ---------------------------------------------------------
TEST(OrcTest, AlMorirSueltaPlataSegunLaProbabilidad) {
    World world(5, 5);
    Player attacker(1, 0, 0);
    auto sure = std::make_unique<Orc>(2, 2, 100);
    auto never = std::make_unique<Orc>(3, 3, 0);
    Orc* sure_ptr = sure.get();
    Orc* never_ptr = never.get();
    world.add_enemy(std::move(sure));
    world.add_enemy(std::move(never));

    for (int hit = 0; hit < 15; ++hit) {  // el orco tiene 15 de vida
        sure_ptr->interact(attacker, ToolType::SWORD, world);
        never_ptr->interact(attacker, ToolType::SWORD, world);
    }

    const auto silver_at = [&world](uint16_t x, uint16_t y) {
        int silver = 0;
        for (const IEntity* ent: world.get_entity_at(x, y)) {
            const auto snap = ent->snapshot();
            if (snap && snap->item == ItemID::SILVER_ORB) {
                ++silver;
            }
        }
        return silver;
    };
    EXPECT_EQ(silver_at(2, 2), 1);  // 100 %: sí suelta plata
    EXPECT_EQ(silver_at(3, 3), 0);  // 0 %: nunca
}

TEST(OrcTest, SpawnOrcPackDejaEntreTresYSeisOrcos) {
    World world(10, 10);
    world.spawn_orc_pack(5, 5);

    int orcs = 0;
    for (uint16_t y = 0; y < world.get_height(); ++y) {
        for (uint16_t x = 0; x < world.get_width(); ++x) {
            for (const IEntity* ent: world.get_entity_at(x, y)) {
                if (dynamic_cast<const Orc*>(ent) != nullptr) {
                    // cppcheck-suppress useStlAlgorithm; además de contar, valida que sea un Orc.
                    ++orcs;
                }
            }
        }
    }
    EXPECT_GE(orcs, 3);
    EXPECT_LE(orcs, 6);
}

TEST(OrcTest, SpawnOrcPackEnLaEsquinaNoSeSaleDelMapa) {
    World world(4, 4);
    world.spawn_orc_pack(0, 0);  // 9 celdas disponibles: entra la manada completa

    int orcs = 0;
    for (uint16_t y = 0; y < world.get_height(); ++y) {
        for (uint16_t x = 0; x < world.get_width(); ++x) {
            for (const IEntity* ent: world.get_entity_at(x, y)) {
                if (dynamic_cast<const Orc*>(ent) != nullptr) {
                    // cppcheck-suppress useStlAlgorithm; además de contar, valida que sea un Orc.
                    ++orcs;
                }
            }
        }
    }
    EXPECT_GE(orcs, 3);
    EXPECT_LE(orcs, 6);
}

// Enemigo de prueba sin IA ni consecuencias: solo fija la probabilidad de oro.
class GoldEnemy: public Enemy {
public:
    GoldEnemy(uint16_t x, uint16_t y, uint8_t gold_percent): Enemy(x, y, 1, 0, gold_percent) {}
    void on_death(World& /*w*/) override {}

protected:
    void behave(World& /*w*/) override {}
};

TEST(EnemyTest, AlMorirSueltaOroSegunLaProbabilidad) {
    World world(5, 5);
    Player attacker(1, 0, 0);
    auto sure = std::make_unique<GoldEnemy>(2, 2, 100);
    auto never = std::make_unique<GoldEnemy>(3, 3, 0);
    GoldEnemy* sure_ptr = sure.get();
    GoldEnemy* never_ptr = never.get();
    world.add_enemy(std::move(sure));
    world.add_enemy(std::move(never));

    sure_ptr->interact(attacker, ToolType::SWORD, world);
    never_ptr->interact(attacker, ToolType::SWORD, world);

    // Cadáver + orbe en la celda con 100 %; solo el cadáver con 0 %.
    ASSERT_EQ(world.get_entity_at(2, 2).size(), 2u);
    EXPECT_EQ(world.get_entity_at(2, 2)[1]->snapshot()->item, ItemID::GOLD_ORB);
    EXPECT_EQ(world.get_entity_at(3, 3).size(), 1u);
}

TEST(EnemyTest, AgregarUnEnemigoFueraDelMapaNoRompe) {
    World world(5, 5);
    world.add_player(1, std::make_unique<Player>(1, 2, 2));
    world.add_enemy(std::make_unique<Skeleton>(7, 7));  // fuera del mapa: se descarta

    auto owned = std::make_unique<Skeleton>(0, 0);
    const Skeleton* skeleton = owned.get();
    world.add_enemy(std::move(owned));
    world.tick_enemies();

    EXPECT_EQ(skeleton->get_x(), 0);  // el válido sigue funcionando: baja un paso
    EXPECT_EQ(skeleton->get_y(), 1);
}
