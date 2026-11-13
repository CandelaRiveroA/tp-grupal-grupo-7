#include <gtest/gtest.h>
#include <memory>

#include "../server/domain/map/world.h"
#include "../server/domain/entities/player.h"
#include "../server/domain/entities/i_entity.h"
#include "../common/common_constants.h"

//TODO : Modularizar la creacion de mundos y jugadores para no repetir tanto código en los tests

// MOCK: Entidad de prueba para simular rocas o árboles
class ParedSolida : public IEntity {
public:
    ParedSolida(uint16_t x, uint16_t y) : IEntity(x, y) {}
    
    // Lo más importante para el test: NO es caminable
    bool is_walkable() const override { return false; }
    void interact(Player& /*p*/, ToolType /*tool*/, World& /*w*/) override {}
    void update(World& /*w*/) override {}};

TEST(WorldTest, JugadorSeMueveHaciaCeldaVacia) {
    World world(5, 5);
    
    auto player = std::make_unique<Player>(1, 2, 2);
    Player* p = player.get();
    world.add_player(1, std::move(player));

    // Movemos a la derecha
    world.move_player(1, Direction::RIGHT);

    // Debe haber avanzado a (3, 2) y mirar a la derecha
    EXPECT_EQ(p->get_x(), 3);
    EXPECT_EQ(p->get_y(), 2);
    EXPECT_EQ(p->get_facing(), Direction::RIGHT);
}

TEST(WorldTest, JugadorNoPuedeSalirDelMapa) {
    World world(5, 5);
    
    auto player = std::make_unique<Player>(1, 0, 0);
    Player* p = player.get();
    world.add_player(1, std::move(player));

    // 1. Intenta salirse por ARRIBA (y=0 -> restarle 1 causaría underflow)
    world.move_player(1, Direction::UP);

    EXPECT_EQ(p->get_x(), 0);
    EXPECT_EQ(p->get_y(), 0);
    EXPECT_EQ(p->get_facing(), Direction::UP);

    // 2. Intenta salirse por la IZQUIERDA (x=0 -> restarle 1 causaría underflow)
    world.move_player(1, Direction::LEFT);

    // Se queda en (0, 0) pero mira a la izquierda
    EXPECT_EQ(p->get_x(), 0);
    EXPECT_EQ(p->get_y(), 0);
    EXPECT_EQ(p->get_facing(), Direction::LEFT);
    
    // 3. Intenta salirse por la DERECHA desde el otro extremo
    p->set_position(4, 4); 
    world.move_player(1, Direction::RIGHT);
    
    EXPECT_EQ(p->get_x(), 4);
    EXPECT_EQ(p->get_y(), 4);
    EXPECT_EQ(p->get_facing(), Direction::RIGHT);
}

// ---------------------------------------------------------
// TEST 3: Colisión contra otras entidades físicas
// ---------------------------------------------------------
TEST(WorldTest, JugadorChocaContraEntidadSolida) {
    World world(5, 5);
    
    auto player = std::make_unique<Player>(1, 2, 2);
    Player* p = player.get();
    world.add_player(1, std::move(player));

    // Colocamos una pared sólida en (3, 2), justo a la derecha del jugador
    world.set_entity_at(3, 2, std::make_unique<ParedSolida>(3, 2));

    // Intentamos caminar hacia la pared
    world.move_player(1, Direction::RIGHT);

    // ¡Colisión! El jugador NO avanza, pero sí gira hacia la pared
    EXPECT_EQ(p->get_x(), 2);
    EXPECT_EQ(p->get_y(), 2);
    EXPECT_EQ(p->get_facing(), Direction::RIGHT);
}