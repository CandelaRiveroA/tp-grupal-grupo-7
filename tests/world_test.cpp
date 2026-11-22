#include <gtest/gtest.h>
#include <memory>

#include "../server/domain/map/world.h"
#include "../server/domain/entities/player.h"
#include "../server/domain/entities/i_entity.h"
#include "../server/domain/inventory/product.h"
#include "../server/domain/inventory/item_types.h"
#include "../server/commands/commands.h"
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

// ---------------------------------------------------------
// TEST 4: agotar la energía desmaya, no mata
// ---------------------------------------------------------
TEST(WorldTest, GastarTodaLaEnergiaDesmayaSinBajarLaVida) {
    World world(5, 5);
    world.add_player(1, std::make_unique<Player>(1, 1, 1));

    // PICKAXE en el slot 0 contra una celda vacía: cada uso gasta 1 de
    // energía. Con 100 de energía hacen falta 100 usos.
    for (int i = 0; i < 200 && !world.get_player(1).faintedByEnergy(); ++i) {
        world.use_tool(1);
    }

    EXPECT_TRUE(world.get_player(1).faintedByEnergy());
    EXPECT_EQ(world.get_player(1).get_health(), 100u);
}

// ---------------------------------------------------------
// TEST 5: el Take con otro jugador cerca pide el regalo
// ---------------------------------------------------------
TEST(WorldTest, TakeJugadorCercanoPideRegalo) {
    World world(5, 5);
    auto giver = std::make_unique<Player>(1, 2, 2);
    Player* p = giver.get();
    world.add_player(1, std::move(giver));
    world.add_player(2, std::make_unique<Player>(2, 3, 2));

    // El Take manda el casillero elegido (la selección es local): con el
    // pico del slot 0 no se puede regalar, no se pide nada.
    commands::apply(world, 1, protocol::TakeMessage{Direction::RIGHT, 0});
    EXPECT_TRUE(world.take_gift_requests().empty());

    // Con una frutilla en ese casillero sí: queda el pedido para el otro
    // jugador, que es a quien Match manda el ServerGiftRequest.
    p->pick_up_item(std::make_unique<Product>(ItemID::STRAWBERRY, 80, Quality::BASE));
    commands::apply(world, 1, protocol::TakeMessage{Direction::RIGHT, 8});

    const auto offers = world.take_gift_requests();
    ASSERT_EQ(offers.size(), 1u);
    EXPECT_EQ(offers[0].sender_id, 1);
    EXPECT_EQ(offers[0].receiver_id, 2);
    EXPECT_EQ(offers[0].item, ItemID::STRAWBERRY);
}

// ---------------------------------------------------------
// TEST 6: el receptor contesta y el emisor recibe el aviso
// ---------------------------------------------------------
TEST(WorldTest, ContestarElRegaloAvisaAlEmisor) {
    World world(5, 5);
    auto giver = std::make_unique<Player>(1, 2, 2);
    Player* p = giver.get();
    world.add_player(1, std::move(giver));
    world.add_player(2, std::make_unique<Player>(2, 3, 2));

    p->pick_up_item(std::make_unique<Product>(ItemID::STRAWBERRY, 80, Quality::BASE));
    commands::apply(world, 1, protocol::TakeMessage{Direction::RIGHT, 8});
    ASSERT_EQ(world.take_gift_requests().size(), 1u);

    // Acepta: se transfiere el ítem y queda el aviso para el emisor
    // (Match lo manda como ServerGiftAnswered y cierra su popup).
    commands::apply(world, 2, protocol::GiftResponseMessage{1, true});
    EXPECT_TRUE(p->get_inventory().is_slot_empty(8));
    auto answers = world.take_gift_answers();
    ASSERT_EQ(answers.size(), 1u);
    EXPECT_EQ(answers[0], 1);

    // Rechaza: no se transfiere nada, pero el aviso sale igual.
    commands::apply(world, 2, protocol::GiftResponseMessage{1, false});
    answers = world.take_gift_answers();
    ASSERT_EQ(answers.size(), 1u);
    EXPECT_EQ(answers[0], 1);
}