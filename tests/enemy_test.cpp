#include <memory>

#include <gtest/gtest.h>

#include "../server/domain/enemies/orc.h"
#include "../server/domain/enemies/skeleton.h"
#include "../server/domain/enemies/skull.h"
#include "../server/domain/enemies/vampire.h"
#include "../server/domain/enemies/vampiress.h"
#include "../server/domain/entities/i_movable.h"
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
    world.set_active_scene("dungeon_1");
    Player attacker(1, 0, 0);
    auto owned = std::make_unique<Skeleton>(2, 2);
    Skeleton* skeleton = owned.get();
    world.add_enemy(std::move(owned));

    EXPECT_FALSE(world.can_walk_to(2, 2));  // vivo: bloquea el paso

    skeleton->interact(attacker, ToolType::PICKAXE, world);
    skeleton->interact(attacker, ToolType::HAND, world);
    EXPECT_EQ(skeleton->get_health(), 10);  // otras herramientas no hacen nada

    for (int hit = 0; hit < 5; ++hit) {  // la espada hace 2 de daño
        skeleton->interact(attacker, ToolType::SWORD, world);
    }
    EXPECT_EQ(skeleton->get_health(), 0);
    EXPECT_TRUE(world.can_walk_to(2, 2));  // cadáver: no bloquea

    skeleton->interact(attacker, ToolType::SWORD, world);  // golpe extra inútil
    EXPECT_EQ(skeleton->get_health(), 0);
}

TEST(EnemyTest, EspadaHaceLaMitadDeDanioConMenosDel20PorCientoDeVida) {
    World world(5, 5);
    world.set_active_scene("dungeon_1");
    Player normal_attacker(1, 0, 0);
    Player wounded_attacker(2, 0, 0);
    wounded_attacker.set_health(19);

    auto normal_skeleton = std::make_unique<Skeleton>(2, 2);
    Skeleton* normal = normal_skeleton.get();
    world.add_enemy(std::move(normal_skeleton));
    normal->interact(normal_attacker, ToolType::SWORD, world);
    EXPECT_EQ(normal->get_health(), 8);

    auto wounded_skeleton = std::make_unique<Skeleton>(3, 2);
    Skeleton* wounded = wounded_skeleton.get();
    world.add_enemy(std::move(wounded_skeleton));
    wounded->interact(wounded_attacker, ToolType::SWORD, world);
    EXPECT_EQ(wounded->get_health(), 9);
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
// Muerte: la animación de desmayo primero, la cama después
// ---------------------------------------------------------

TEST(EnemyTest, ElMuertoEsperaLaAnimacionAntesDeIrALaCama) {
    World world(25, 15);
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 1, 1, "dungeon_1", "farmhouse", 20, 13));
    world.add_player(2, std::make_unique<Player>(2, 4, 4, "dungeon_1"));  // no duerme: el día no avanza
    world.damage_player(1, 100);  // mata

    // La animación de desmayo corre donde cayó: mientras espera no lo
    // teletransporta ni lo registra para dormir.
    EXPECT_FALSE(world.is_sleeping(1));
    bool arrived = false;
    for (int steps = 0; steps < 10 && !arrived; ++steps) {
        arrived = world.tick_death_to_bed();
        if (!arrived) {
            EXPECT_EQ(world.get_player(1).get_x(), 1);
            EXPECT_EQ(world.get_player(1).get_y(), 1);
        }
    }

    // Al vencer la espera va a la cama con el sueño registrado: el resto
    // del flujo (pregunta, avance del día, amanecer) es el de siempre.
    EXPECT_TRUE(arrived);
    EXPECT_EQ(world.get_player(1).get_x(), 20);
    EXPECT_EQ(world.get_player(1).get_y(), 13);
    EXPECT_TRUE(world.is_sleeping(1));
    EXPECT_FALSE(world.is_sleeping(2));
}

TEST(EnemyTest, SiReviveMientrasEsperaNoLoMandanALaCama) {
    World world(5, 5);
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 1, 1, "dungeon_1"));
    world.add_player(2, std::make_unique<Player>(2, 4, 4, "dungeon_1"));
    world.damage_player(1, 100);  // mata
    world.reset_player_for_day(1);  // amaneció mientras esperaba: revive

    for (int steps = 0; steps < 10; ++steps) {
        world.tick_death_to_bed();
    }

    // No lo manda a la cama ni lo registra: ya no está muerto.
    EXPECT_EQ(world.get_player(1).get_x(), 1);
    EXPECT_EQ(world.get_player(1).get_y(), 1);
    EXPECT_FALSE(world.is_sleeping(1));
}

// ---------------------------------------------------------
// Revive del esqueleto (estado muerto en la celda)
// ---------------------------------------------------------
TEST(EnemyTest, ElEsqueletoReviveConVidaLlenaDespuesDeLosTicks) {
    World world(5, 5);
    world.set_active_scene("dungeon_1");
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
    world.set_active_scene("dungeon_1");
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
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 5, 1, "dungeon_1"));
    auto owned = std::make_unique<Skeleton>(1, 1);
    const Skeleton* skeleton = owned.get();
    world.add_enemy(std::move(owned));

    world.tick_enemies_pursuit();

    EXPECT_EQ(skeleton->get_x(), 2);  // un paso hacia el jugador
    EXPECT_EQ(skeleton->get_y(), 1);
    EXPECT_EQ(world.get_entity_at(2, 1).size(), 1u);
    EXPECT_EQ(world.get_entity_at(1, 1).size(), 0u);
}

TEST(EnemyTest, ElEsqueletoGolpeaAlJugadorAdyacente) {
    World world(10, 10);
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 2, 1, "dungeon_1"));
    auto owned = std::make_unique<Skeleton>(1, 1);
    world.add_enemy(std::move(owned));

    world.tick_enemies_pursuit();

    EXPECT_EQ(world.get_player(1).get_health(), 95);  // 5 de daño cuerpo a cuerpo
}

TEST(EnemyTest, ElEsqueletoIgnoraAlJugadorFueraDeRango) {
    World world(10, 10);
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 9, 9, "dungeon_1"));
    auto owned = std::make_unique<Skeleton>(0, 0);
    const Skeleton* skeleton = owned.get();
    world.add_enemy(std::move(owned));

    world.tick_enemies_pursuit();

    EXPECT_EQ(skeleton->get_x(), 0);  // fuera de rango (distancia 9 > 5)
    EXPECT_EQ(skeleton->get_y(), 0);
    EXPECT_EQ(world.get_player(1).get_health(), 100);
}

TEST(EnemyTest, ElEsqueletoNoPersigueAUnJugadorMuerto) {
    World world(10, 10);
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 1, 2, "dungeon_1"));
    world.add_player(2, std::make_unique<Player>(2, 4, 1, "dungeon_1"));
    world.damage_player(1, 100);
    auto owned = std::make_unique<Skeleton>(1, 1);
    const Skeleton* skeleton = owned.get();
    world.add_enemy(std::move(owned));

    world.tick_enemies_pursuit();

    EXPECT_EQ(skeleton->get_x(), 2);  // va hacia el vivo, no pega al muerto
    EXPECT_EQ(skeleton->get_y(), 1);
    EXPECT_EQ(world.get_player(1).get_health(), 0);
}

TEST(IMovableTest, NextTileAvanzaYSeQuedaEnElBorde) {
    using Tile = std::pair<uint16_t, uint16_t>;
    EXPECT_EQ(IMovable::next_tile(3, 3, Direction::UP), (Tile{3, 2}));
    EXPECT_EQ(IMovable::next_tile(3, 3, Direction::DOWN), (Tile{3, 4}));
    EXPECT_EQ(IMovable::next_tile(3, 3, Direction::LEFT), (Tile{2, 3}));
    EXPECT_EQ(IMovable::next_tile(3, 3, Direction::RIGHT), (Tile{4, 3}));
    EXPECT_EQ(IMovable::next_tile(0, 0, Direction::UP), (Tile{0, 0}));
    EXPECT_EQ(IMovable::next_tile(0, 0, Direction::LEFT), (Tile{0, 0}));
}

// ---------------------------------------------------------
// Orc
// ---------------------------------------------------------
TEST(OrcTest, AlMorirSueltaPlataSegunLaProbabilidad) {
    World world(5, 5);
    world.set_active_scene("dungeon_1");
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
    world.set_active_scene("dungeon_1");
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
    world.set_active_scene("dungeon_1");
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
    std::string kind_name() const override { return "gold"; }

protected:
    void behave(World& /*w*/) override {}
};

TEST(EnemyTest, AlMorirSueltaOroSegunLaProbabilidad) {
    World world(5, 5);
    world.set_active_scene("dungeon_1");
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
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 2, 2, "dungeon_1"));
    world.add_enemy(std::make_unique<Skeleton>(7, 7));  // fuera del mapa: se descarta

    auto owned = std::make_unique<Skeleton>(0, 0);
    const Skeleton* skeleton = owned.get();
    world.add_enemy(std::move(owned));
    world.tick_enemies_pursuit();

    EXPECT_EQ(skeleton->get_x(), 0);  // el válido sigue funcionando: baja un paso
    EXPECT_EQ(skeleton->get_y(), 1);
}

// ---------------------------------------------------------
// Vampiro: sangre, revive y frenesí de pareja
// ---------------------------------------------------------
TEST(VampireTest, ElGolpeGraveDejaCharcoYElVampiroReviveUnaVez) {
    World world(10, 10);
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 0, 0, "dungeon_1"));
    Player attacker(1, 0, 0);
    world.damage_player(1, 85);  // 100 -> 15: por debajo del 20 %, deja sangre

    auto owned = std::make_unique<Vampire>(3, 3);
    Vampire* vampire = owned.get();
    world.add_enemy(std::move(owned));

    for (int hit = 0; hit < 30; ++hit) {  // el vampiro tiene 30 de vida
        vampire->interact(attacker, ToolType::SWORD, world);
    }
    ASSERT_EQ(vampire->get_health(), 0);

    for (int tick = 0; tick < 10; ++tick) {  // 10 ticks: contador a 0 y revive
        world.tick_enemies();
    }
    EXPECT_EQ(vampire->get_health(), 30);  // revivió consumiendo el charco

    // El charco se consumió: la segunda muerte no revive.
    for (int hit = 0; hit < 30; ++hit) {
        vampire->interact(attacker, ToolType::SWORD, world);
    }
    ASSERT_EQ(vampire->get_health(), 0);
    for (int tick = 0; tick < 15; ++tick) {
        world.tick_enemies();
    }
    EXPECT_EQ(vampire->get_health(), 0);
}

TEST(VampireTest, LaMuerteDelJugadorTambienDejaCharco) {
    World world(10, 10);
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 0, 0, "dungeon_1"));
    Player attacker(1, 0, 0);
    world.damage_player(1, 100);  // mata al jugador: también deja sangre

    auto owned = std::make_unique<Vampire>(3, 3);
    Vampire* vampire = owned.get();
    world.add_enemy(std::move(owned));

    for (int hit = 0; hit < 30; ++hit) {
        vampire->interact(attacker, ToolType::SWORD, world);
    }
    ASSERT_EQ(vampire->get_health(), 0);

    for (int tick = 0; tick < 10; ++tick) {
        world.tick_enemies();
    }
    EXPECT_EQ(vampire->get_health(), 30);  // revivió con el charco de la muerte
}

TEST(VampireTest, UnGolpeNoGraveNoDejaCharcoYElVampiroNoRevive) {
    World world(10, 10);
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 0, 0, "dungeon_1"));
    Player attacker(1, 0, 0);
    world.damage_player(1, 40);  // queda en 60: no es grave, no hay sangre

    auto owned = std::make_unique<Vampire>(3, 3);
    Vampire* vampire = owned.get();
    world.add_enemy(std::move(owned));

    for (int hit = 0; hit < 30; ++hit) {
        vampire->interact(attacker, ToolType::SWORD, world);
    }
    ASSERT_EQ(vampire->get_health(), 0);

    for (int tick = 0; tick < 20; ++tick) {  // pasó el plazo de revive
        world.tick_enemies();
    }
    EXPECT_EQ(vampire->get_health(), 0);
}

TEST(VampireTest, AlMorirLaVampiraElVampiroEnfureceYPegaElDoble) {
    World world(10, 10);
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 5, 5, "dungeon_1"));
    Player attacker(1, 0, 0);
    auto male = std::make_unique<Vampire>(6, 5);  // al lado del jugador
    auto female = std::make_unique<Vampiress>(8, 8);
    Vampire* male_ptr = male.get();
    Vampiress* female_ptr = female.get();
    male_ptr->set_partner(female_ptr);
    female_ptr->set_partner(male_ptr);
    world.add_enemy(std::move(male));
    world.add_enemy(std::move(female));

    EXPECT_EQ(male_ptr->get_damage(), 15);  // sin frenesí
    for (int hit = 0; hit < 20; ++hit) {  // la vampira tiene 20 de vida
        female_ptr->interact(attacker, ToolType::SWORD, world);
    }
    ASSERT_EQ(female_ptr->get_health(), 0);
    EXPECT_EQ(male_ptr->get_damage(), 30);  // muerte de la pareja: daño x2

    world.tick_enemies_pursuit();
    EXPECT_EQ(world.get_player(1).get_health(), 70);  // 30 del golpe frenético
}

TEST(VampireTest, AlMorirElVampiroLaVampiraEnfureceYCorreElDoble) {
    World world(10, 10);
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 0, 0, "dungeon_1"));
    Player attacker(1, 0, 0);
    auto male = std::make_unique<Vampire>(8, 0);
    auto female = std::make_unique<Vampiress>(5, 0);  // a 5 del jugador
    Vampire* male_ptr = male.get();
    Vampiress* female_ptr = female.get();
    male_ptr->set_partner(female_ptr);
    female_ptr->set_partner(male_ptr);
    world.add_enemy(std::move(male));
    world.add_enemy(std::move(female));

    for (int hit = 0; hit < 30; ++hit) {  // el vampiro tiene 30 de vida
        male_ptr->interact(attacker, ToolType::SWORD, world);
    }
    ASSERT_EQ(male_ptr->get_health(), 0);

    world.tick_enemies_pursuit();
    // 2 de speed x 2 de frenesí = 4 pasos: de 5 a 1 (sin frenesí serían 2).
    EXPECT_EQ(female_ptr->get_x(), 1);
    EXPECT_EQ(female_ptr->get_y(), 0);
}

TEST(VampireTest, SpawnVampirePairDejaLaParejaEnlazada) {
    World world(10, 10);
    world.set_active_scene("dungeon_1");
    world.spawn_vampire_pair(5, 5);

    Vampire* male = nullptr;
    Vampiress* female = nullptr;
    for (uint16_t y = 0; y < world.get_height(); ++y) {
        for (uint16_t x = 0; x < world.get_width(); ++x) {
            for (IEntity* ent: world.get_entity_at(x, y)) {
                if (auto* vampiress = dynamic_cast<Vampiress*>(ent)) {
                    female = vampiress;
                } else if (auto* vampire = dynamic_cast<Vampire*>(ent)) {
                    male = vampire;
                }
            }
        }
    }
    ASSERT_NE(male, nullptr);
    ASSERT_NE(female, nullptr);

    // El enlace mutuo se proba por el frenesí: al morir la vampira, el
    // vampiro pasa a pegar el doble.
    Player attacker(1, 0, 0);
    EXPECT_EQ(male->get_damage(), 15);
    for (int hit = 0; hit < 20; ++hit) {
        female->interact(attacker, ToolType::SWORD, world);
    }
    EXPECT_EQ(female->get_health(), 0);
    EXPECT_EQ(male->get_damage(), 30);
}

TEST(VampireTest, SiLaParejaSaleDelMundoElSobrevivienteNoQuedaColgado) {
    World world(10, 10);
    world.set_active_scene("dungeon_1");
    world.spawn_vampire_pair(5, 5);

    Vampire* male = nullptr;
    Vampiress* female = nullptr;
    for (uint16_t y = 0; y < world.get_height(); ++y) {
        for (uint16_t x = 0; x < world.get_width(); ++x) {
            for (IEntity* ent: world.get_entity_at(x, y)) {
                if (auto* vampiress = dynamic_cast<Vampiress*>(ent)) {
                    female = vampiress;
                } else if (auto* vampire = dynamic_cast<Vampire*>(ent)) {
                    male = vampire;
                }
            }
        }
    }
    ASSERT_NE(male, nullptr);
    ASSERT_NE(female, nullptr);

    world.remove_entity(female);

    // Sin el desenlace en ~Vampire, esta muerte tocaría la vampira ya borrada.
    EXPECT_NO_THROW(male->on_death(world));
}

TEST(VampireTest, FinDeDiaLimpiaLosCharcos) {
    World world(10, 10);
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 0, 0, "dungeon_1"));
    Player attacker(1, 0, 0);
    world.damage_player(1, 85);  // deja un charco...
    world.clear_blood_puddle();  // ...que el fin de día seca

    auto owned = std::make_unique<Vampire>(3, 3);
    Vampire* vampire = owned.get();
    world.add_enemy(std::move(owned));

    for (int hit = 0; hit < 30; ++hit) {
        vampire->interact(attacker, ToolType::SWORD, world);
    }
    ASSERT_EQ(vampire->get_health(), 0);

    for (int tick = 0; tick < 15; ++tick) {  // sin sangre no revive
        world.tick_enemies();
    }
    EXPECT_EQ(vampire->get_health(), 0);
}

// ---------------------------------------------------------
// Skull: invulnerable y de llamas a distancia (proyectil)
// ---------------------------------------------------------
TEST(SkullTest, NoPuedeSerAsesinadoNiSueltaOro) {
    World world(10, 10);
    world.set_active_scene("dungeon_1");
    Player attacker(1, 0, 0);
    auto owned = std::make_unique<Skull>(3, 3);
    Skull* skull = owned.get();
    world.add_enemy(std::move(owned));

    for (int hit = 0; hit < 30; ++hit) {
        skull->interact(attacker, ToolType::SWORD, world);
    }
    EXPECT_EQ(skull->get_health(), 50);
    EXPECT_FALSE(world.can_walk_to(3, 3));  // sigue bloqueando la celda
    EXPECT_EQ(world.get_entity_at(3, 3).size(), 1u);  // solo el skull, sin orbe
}

TEST(SkullTest, LanzaLlamaQueVuelaHaciaElJugadorSinMoverse) {
    World world(10, 10);
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 3, 6, "dungeon_1"));  // a distancia 3
    auto owned = std::make_unique<Skull>(3, 3);
    Skull* skull = owned.get();
    world.add_enemy(std::move(owned));

    world.tick_enemies();  // el reloj lento lanza la llama
    EXPECT_EQ(skull->get_x(), 3);
    EXPECT_EQ(skull->get_y(), 3);
    ASSERT_EQ(world.projectile_snapshots().size(), 1u);
    EXPECT_EQ(world.projectile_snapshots()[0].kind, "skull_fire");
    EXPECT_EQ(world.get_player(1).get_health(), 100);  // el daño lo da el vuelo

    world.tick_flames();  // un paso de vuelo (400 ms)
    EXPECT_EQ(world.projectile_snapshots()[0].y, 4);
    EXPECT_EQ(world.get_player(1).get_health(), 100);

    world.tick_flames();
    world.tick_flames();  // tercero: pega en la celda del jugador
    EXPECT_EQ(world.get_player(1).get_health(), 95);  // 5 de daño al contacto
    EXPECT_TRUE(world.projectile_snapshots().empty());  // se apagó al pegar
}

TEST(SkullTest, FueraDeRangoNoLanzaLlama) {
    World world(10, 10);
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 3, 11, "dungeon_1"));  // a distancia 8
    world.add_enemy(std::make_unique<Skull>(3, 3));

    world.tick_enemies();
    EXPECT_TRUE(world.projectile_snapshots().empty());
    EXPECT_EQ(world.get_player(1).get_health(), 100);
}

TEST(SkullTest, LaLlamaVerdeSaleCadaDosTicks) {
    World world(10, 10);
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 3, 6, "dungeon_1"));
    world.add_enemy(std::make_unique<Skull>(3, 3));

    world.tick_enemies();  // tick 1: solo azul
    ASSERT_EQ(world.projectile_snapshots().size(), 1u);
    EXPECT_EQ(world.projectile_snapshots()[0].kind, "skull_fire");

    world.tick_enemies();  // tick 2: azul + verde
    const std::vector<ProjectileSnapshot> projectiles = world.projectile_snapshots();
    ASSERT_EQ(projectiles.size(), 3u);  // las dos anteriores siguen volando
    EXPECT_EQ(projectiles[2].kind, "skull_fire_green");
    EXPECT_EQ(world.get_player(1).get_health(), 100);
}

TEST(SkullTest, SeAcercaAlJugadorHastaLaDistanciaDeTiro) {
    World world(10, 10);
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 3, 9, "dungeon_1"));
    auto owned = std::make_unique<Skull>(3, 3);
    const Skull* skull = owned.get();
    world.add_enemy(std::move(owned));

    world.tick_enemies_pursuit();  // distancia 4 > 3: da un paso
    EXPECT_EQ(skull->get_x(), 3);
    EXPECT_EQ(skull->get_y(), 4);

    world.tick_enemies_pursuit();  // distancia 3: ya está a tiro y se queda
    EXPECT_EQ(skull->get_x(), 3);
    EXPECT_EQ(skull->get_y(), 4);

    world.tick_enemies();  // el reloj lento dispara, no deambula
    EXPECT_EQ(skull->get_x(), 3);
    EXPECT_EQ(skull->get_y(), 4);
}

TEST(SkullTest, FueraDeRangoDeAggroNoSeMueve) {
    World world(10, 10);
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 9, 9, "dungeon_1"));  // distancia 9 > 7
    auto owned = std::make_unique<Skull>(0, 0);
    const Skull* skull = owned.get();
    world.add_enemy(std::move(owned));

    world.tick_enemies_pursuit();

    EXPECT_EQ(skull->get_x(), 0);
    EXPECT_EQ(skull->get_y(), 0);
}


// ---------------------------------------------------------
// Llamas: proyectil que vuela, pega al contacto y no bloquea
// ---------------------------------------------------------
TEST(FlameTest, LaLlamaPegaAlContactoYSeApaga) {
    World world(10, 10);
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 4, 1, "dungeon_1"));
    world.spawn_flame(4, 4, 4, 1, 5, "skull_fire");  // sube hacia el jugador

    ASSERT_EQ(world.projectile_snapshots().size(), 1u);
    for (int step = 0; step < 3; ++step) {
        world.tick_flames();
    }
    EXPECT_EQ(world.get_player(1).get_health(), 95);
    EXPECT_TRUE(world.projectile_snapshots().empty());
}

TEST(FlameTest, LaLlamaMuereContraUnaPared) {
    World world(5, 5);
    world.set_active_scene("dungeon_1");
    world.get_scene("dungeon_1").get_cell(2, 1).is_wall = true;  // el objetivo está del otro lado
    world.spawn_flame(2, 2, 2, 0, 5, "skull_fire");

    world.tick_flames();
    EXPECT_TRUE(world.projectile_snapshots().empty());
    EXPECT_EQ(world.get_entity_at(2, 1).size(), 0u);
}

TEST(FlameTest, LaLlamaMuereContraElBordeDelMapa) {
    World world(5, 5);
    world.set_active_scene("dungeon_1");
    world.spawn_flame(0, 4, 0, 9, 5, "skull_fire");  // objetivo fuera del mapa

    world.tick_flames();
    EXPECT_TRUE(world.projectile_snapshots().empty());
}

TEST(FlameTest, LaLlamaNoBloqueaLasCeldas) {
    World world(5, 5);
    world.set_active_scene("dungeon_1");
    world.spawn_flame(2, 2, 2, 4, 5, "skull_fire");

    EXPECT_TRUE(world.can_walk_to(2, 2));
    EXPECT_TRUE(world.get_entity_at(2, 2).empty());
}

// ---------------------------------------------------------
// Cadencia de persecución (reloj rápido de 400 ms)
// ---------------------------------------------------------
TEST(EnemyTest, ElGolpeTieneCooldownDeDiezTicksDePersecucion) {
    World world(10, 10);
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 2, 1, "dungeon_1"));
    world.add_enemy(std::make_unique<Skeleton>(1, 1));  // adyacente

    world.tick_enemies_pursuit();
    EXPECT_EQ(world.get_player(1).get_health(), 95);  // golpea de entrada

    for (int tick = 0; tick < 9; ++tick) {  // 9 ticks esperando el cooldown
        world.tick_enemies_pursuit();
    }
    EXPECT_EQ(world.get_player(1).get_health(), 95);

    world.tick_enemies_pursuit();  // décimo: vuelve a pegar (4 s después)
    EXPECT_EQ(world.get_player(1).get_health(), 90);
}

TEST(EnemyTest, SinJugadorEnRangoDeambulaSinSalirseDelMapa) {
    World world(5, 5);
    world.set_active_scene("dungeon_1");
    auto owned = std::make_unique<Skeleton>(2, 2);
    const Skeleton* skeleton = owned.get();
    world.add_enemy(std::move(owned));

    for (int tick = 0; tick < 30; ++tick) {
        world.tick_enemies();
    }
    EXPECT_LT(skeleton->get_x(), 5);
    EXPECT_LT(skeleton->get_y(), 5);
    EXPECT_EQ(world.get_entity_at(skeleton->get_x(), skeleton->get_y()).size(), 1u);
}

TEST(EnemyTest, NoDeambulaMientrasHayUnJugadorEnRango) {
    World world(5, 5);
    world.set_active_scene("dungeon_1");
    world.add_player(1, std::make_unique<Player>(1, 0, 4, "dungeon_1"));  // en rango de aggro
    auto owned = std::make_unique<Skeleton>(0, 0);
    const Skeleton* skeleton = owned.get();
    world.add_enemy(std::move(owned));

    world.tick_enemies();  // reloj lento: sin persecución ni deambular
    EXPECT_EQ(skeleton->get_x(), 0);
    EXPECT_EQ(skeleton->get_y(), 0);
    EXPECT_EQ(world.get_player(1).get_health(), 100);
}
