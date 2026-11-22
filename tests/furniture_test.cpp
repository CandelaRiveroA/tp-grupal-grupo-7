#include <memory>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "../common/common_constants.h"
#include "../server/domain/animals/chicken.h"
#include "../server/domain/entities/player.h"
#include "../server/domain/furniture/bed.h"
#include "../server/domain/inventory/decoration_item.h"
#include "../server/domain/inventory/item.h"
#include "../server/domain/map/world.h"

#include "world_view.h"

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

bool has_item(const Player& player, ItemID id) {
    const Inventory& inventory = player.get_inventory();
    for (uint8_t slot = 0; slot < inventory.get_max_slots(); ++slot) {
        const Item* item = inventory.peek_item(slot);
        if (item != nullptr && item->get_id() == id) {
            return true;
        }
    }
    return false;
}

bool cell_has_item(const World& world, uint16_t x, uint16_t y, ItemID id) {
    // cppcheck-suppress useStlAlgorithm;
    for (const IEntity* entity: world.get_entity_at(x, y)) {
        const auto snap = entity->snapshot();
        if (snap && snap->item == id) {
            return true;
        }
    }
    return false;
}

// La cama es caminable: no se distingue de una celda vacía por las
// colisiones, hay que mirar su snapshot.
bool cell_has_furniture(const World& world, uint16_t x, uint16_t y) {
    // cppcheck-suppress useStlAlgorithm;
    for (const IEntity* entity: world.get_entity_at(x, y)) {
        const auto snap = entity->snapshot();
        if (snap && snap->kind == CellSnapshot::Kind::Furniture) {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------
// La cama se muestra como mueble, no como un ítem del piso.
// ---------------------------------------------------------
TEST(FurnitureTest, LaCamaSeMuestraComoMuebleEnElSync) {
    World world(5, 5);
    world.add_player(1, std::make_unique<Player>(1, 0, 0));
    world.set_entity_at(2, 2, std::make_unique<Bed>(2, 2));
    WorldView view;

    const std::vector<ServerMessage> messages = view.sync(world, 1);

    const std::vector<ServerFurniture> furniture = only<ServerFurniture>(messages);
    ASSERT_EQ(furniture.size(), 1u);
    EXPECT_EQ(furniture[0].x, 2);
    EXPECT_EQ(furniture[0].y, 2);
    EXPECT_EQ(furniture[0].content_id, "bed");

    // El cliente no debe tratarla como un ítem del piso recogible.
    EXPECT_TRUE(only<ServerDropped>(messages).empty());
}

// ---------------------------------------------------------
// Un golpe de pico la destruye y deja el DecorationItem en el piso.
// ---------------------------------------------------------
TEST(FurnitureTest, UnGolpeDePicoLaDestruyeYDejaElDecorationItem) {
    World world(5, 5);
    auto player = std::make_unique<Player>(1, 1, 2);
    Player* p = player.get();
    world.add_player(1, std::move(player));
    world.set_entity_at(1, 3, std::make_unique<Bed>(1, 3));
    world.set_player_facing(1, Direction::DOWN);

    // La pala (slot 1) no la rompe.
    p->select_slot(1);
    world.use_tool(1);
    EXPECT_TRUE(cell_has_furniture(world, 1, 3));

    // El pico (slot 0): un golpe y se va, dejando un drop pisable.
    p->select_slot(0);
    world.use_tool(1);
    EXPECT_FALSE(cell_has_furniture(world, 1, 3));
    EXPECT_TRUE(cell_has_item(world, 1, 3, ItemID::BED));
    EXPECT_TRUE(world.can_walk_to(1, 3));

    // El jugador camina sobre el drop y lo recoge (efecto imán).
    world.move_player(1, Direction::DOWN);
    EXPECT_EQ(p->get_x(), 1);
    EXPECT_EQ(p->get_y(), 4);
    EXPECT_TRUE(has_item(*p, ItemID::BED));

    const std::vector<PlayerPickup> pickups = world.take_pickups();
    ASSERT_EQ(pickups.size(), 1u);
    EXPECT_EQ(pickups[0].item, ItemID::BED);
}

// ---------------------------------------------------------
// El DecorationItem se apila por ID, como cualquier ítem base.
// ---------------------------------------------------------
TEST(FurnitureTest, ElDecorationItemSeApilaPorId) {
    const DecorationItem bed_a(ItemID::BED, 0);
    const DecorationItem bed_b(ItemID::BED, 0);

    EXPECT_TRUE(bed_a.is_stackable_with(bed_b));
}

// ---------------------------------------------------------
// Pisar la cama pide dormir, bloquea el movimiento y cancelar lo devuelve.
// ---------------------------------------------------------
TEST(FurnitureTest, PisarLaCamaPideDormirYBloqueaElMovimiento) {
    World world(5, 5);
    auto player = std::make_unique<Player>(1, 2, 1);
    const Player* p = player.get();
    world.add_player(1, std::move(player));
    world.set_entity_at(2, 2, std::make_unique<Bed>(2, 2));

    // La cama se pisa: el jugador se superpone a su celda.
    EXPECT_TRUE(world.can_walk_to(2, 2));
    world.move_player(1, Direction::DOWN);
    EXPECT_EQ(p->get_x(), 2);
    EXPECT_EQ(p->get_y(), 2);
    EXPECT_EQ(world.sleeping_count(), 1);

    // Durmiendo no se mueve.
    world.move_player(1, Direction::LEFT);
    EXPECT_EQ(p->get_x(), 2);
    EXPECT_EQ(p->get_y(), 2);

    // Cancelar le devuelve el movimiento.
    world.answer_sleep(1, false);
    EXPECT_EQ(world.sleeping_count(), 0);
    world.move_player(1, Direction::LEFT);
    EXPECT_EQ(p->get_x(), 0);
    EXPECT_EQ(p->get_y(), 2);
}

// ---------------------------------------------------------
// Todos los jugadores deben aceptar para dormir; al dormir, la gallina
// produce aunque no haya pasado el día completo.
// ---------------------------------------------------------
TEST(FurnitureTest, TodosLosJugadoresDebenAceptarParaDormir) {
    World world(6, 6);
    world.add_player(1, std::make_unique<Player>(1, 3, 2));
    world.add_player(2, std::make_unique<Player>(2, 5, 3));
    world.add_animal(std::make_unique<Chicken>(0, 0));
    world.set_entity_at(3, 3, std::make_unique<Bed>(3, 3));

    world.move_player(1, Direction::DOWN);
    world.move_player(2, Direction::LEFT);
    world.move_player(2, Direction::LEFT);
    ASSERT_EQ(world.sleeping_count(), 2);

    // El primero acepta: falta el segundo.
    world.answer_sleep(1, true);
    EXPECT_FALSE(world.take_day_ended());

    // El segundo acepta: termina el día, pero todavía no arranca.
    world.answer_sleep(2, true);
    EXPECT_TRUE(world.take_day_ended());
    EXPECT_FALSE(world.take_day_ended());
    EXPECT_FALSE(world.take_day_started());
    EXPECT_EQ(world.sleeping_count(), 0);

    // Arranca recién cuando despiertan los dos; ahí produce la gallina.
    world.answer_wake(1);
    EXPECT_FALSE(world.take_day_started());
    world.answer_wake(2);
    EXPECT_TRUE(world.take_day_started());
    EXPECT_FALSE(world.take_day_started());
    EXPECT_TRUE(cell_has_item(world, 0, 0, ItemID::EGG));
}

// ---------------------------------------------------------
// Cancelar solo borra el OK del que cancela: los demás no tienen que repetir.
// ---------------------------------------------------------
TEST(FurnitureTest, CancelarSoloBorraSuOK) {
    World world(6, 6);
    world.add_player(1, std::make_unique<Player>(1, 3, 2));
    world.add_player(2, std::make_unique<Player>(2, 3, 4));
    world.add_player(3, std::make_unique<Player>(3, 5, 3));
    world.set_entity_at(3, 3, std::make_unique<Bed>(3, 3));

    world.move_player(1, Direction::DOWN);
    world.move_player(2, Direction::UP);
    world.move_player(3, Direction::LEFT);
    world.move_player(3, Direction::LEFT);
    ASSERT_EQ(world.sleeping_count(), 3);

    world.answer_sleep(1, true);
    world.answer_sleep(2, true);
    EXPECT_FALSE(world.take_day_ended());

    // El tercero cancela: sale de la cama, pero los OK de los otros se mantienen.
    world.answer_sleep(3, false);
    EXPECT_EQ(world.sleeping_count(), 2);

    // El tercero reentra (está parado en la cama: con la mano se vuelve a
    // pedir dormir) y acepta: con los OK de 1 y 2 intactos, el día termina.
    world.take_item(3);
    ASSERT_EQ(world.sleeping_count(), 3);
    EXPECT_FALSE(world.take_day_ended());

    world.answer_sleep(3, true);
    EXPECT_TRUE(world.take_day_ended());
}

// ---------------------------------------------------------
// La desconexión de un jugador no deja la votación colgada.
// ---------------------------------------------------------
TEST(FurnitureTest, DesconectarUnJugadorNoDejaLaVotacionColgada) {
    World world(6, 6);
    world.add_player(1, std::make_unique<Player>(1, 3, 2));
    world.add_player(2, std::make_unique<Player>(2, 5, 3));
    world.set_entity_at(3, 3, std::make_unique<Bed>(3, 3));

    world.move_player(1, Direction::DOWN);
    world.move_player(2, Direction::LEFT);
    world.move_player(2, Direction::LEFT);
    ASSERT_EQ(world.sleeping_count(), 2);

    // Se va el segundo: el primero ya no espera a nadie.
    world.remove_player(2);
    EXPECT_EQ(world.sleeping_count(), 1);

    world.answer_sleep(1, true);
    EXPECT_TRUE(world.take_day_ended());
}

// ---------------------------------------------------------
// Si el único que faltaba aceptar se va, el día avanza igual.
// ---------------------------------------------------------
TEST(FurnitureTest, SiSeVaElQueFaltabaAceptarAvanzaElDia) {
    World world(6, 6);
    world.add_player(1, std::make_unique<Player>(1, 3, 2));
    world.add_player(2, std::make_unique<Player>(2, 3, 4));
    world.add_player(3, std::make_unique<Player>(3, 5, 3));
    world.set_entity_at(3, 3, std::make_unique<Bed>(3, 3));

    world.move_player(1, Direction::DOWN);
    world.move_player(2, Direction::UP);
    world.move_player(3, Direction::LEFT);
    world.move_player(3, Direction::LEFT);
    ASSERT_EQ(world.sleeping_count(), 3);

    world.answer_sleep(1, true);
    world.answer_sleep(2, true);
    EXPECT_FALSE(world.take_day_ended());

    world.remove_player(3);
    EXPECT_TRUE(world.take_day_ended());
}

// ---------------------------------------------------------
// El día terminado (por timeout) espera a que todos despierten; si uno se
// va mientras tanto, los demás no quedan trabados.
// ---------------------------------------------------------
TEST(FurnitureTest, DiaTerminadoEsperaQueDespiertenTodos) {
    World world(6, 6);
    world.add_player(1, std::make_unique<Player>(1, 3, 2));
    world.add_player(2, std::make_unique<Player>(2, 3, 4));
    world.add_player(3, std::make_unique<Player>(3, 5, 3));

    world.end_day();
    EXPECT_TRUE(world.take_day_ended());
    EXPECT_TRUE(world.is_waking());

    world.answer_wake(1);
    EXPECT_FALSE(world.take_day_started());

    world.remove_player(3);
    EXPECT_FALSE(world.take_day_started());

    world.answer_wake(2);
    EXPECT_TRUE(world.take_day_started());
    EXPECT_FALSE(world.is_waking());
}

}  // namespace
