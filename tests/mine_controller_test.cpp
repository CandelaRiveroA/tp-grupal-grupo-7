#include "../server/domain/minerals/mine_controller.h"

#include <memory>
#include <utility>

#include <gtest/gtest.h>

#include "../server/domain/entities/dropped_item.h"
#include "../server/domain/furniture/bed.h"
#include "../server/domain/inventory/product.h"
#include "../server/domain/map/world.h"

namespace {

// Cuenta las celdas cuyo snapshot tiene el tipo pedido. El snapshot es la
// forma que tiene el mundo de describir una entidad sin conocer su clase.
size_t count_kind(const World& world, const CellSnapshot::Kind kind) {
    size_t count = 0;
    for (uint16_t y = 0; y < world.get_height(); ++y) {
        for (uint16_t x = 0; x < world.get_width(); ++x) {
            for (const IEntity* entity: world.get_entity_at(x, y)) {
                const auto snap = entity->snapshot();
                if (snap && snap->kind == kind) {
                    ++count;
                }
            }
        }
    }
    return count;
}

// Minerales que dropean `item` (el oro es GOLD_ORB, la roca ROCK).
size_t count_resource_with(const World& world, const ItemID item) {
    size_t count = 0;
    for (uint16_t y = 0; y < world.get_height(); ++y) {
        for (uint16_t x = 0; x < world.get_width(); ++x) {
            for (const IEntity* entity: world.get_entity_at(x, y)) {
                const auto snap = entity->snapshot();
                if (snap && snap->kind == CellSnapshot::Kind::Resource && snap->item == item) {
                    ++count;
                }
            }
        }
    }
    return count;
}

}  // namespace

TEST(MineControllerTest, GeneraElPorcentajeDeCeldasLibres) {
    World world(30, 20);
    MineController mine(1, 42);
    mine.populate(world, 15, 10);

    // 600 celdas menos la reservada = 599; el 10% es 59.
    EXPECT_EQ(count_kind(world, CellSnapshot::Kind::Resource), 59u);
    // La celda reservada (aparición del jugador) queda libre.
    EXPECT_TRUE(world.get_entity_at(15, 10).empty());
}

TEST(MineControllerTest, SalaUnoNoGeneraOro) {
    World world(30, 20);
    MineController mine(1, 42);
    mine.populate(world, 15, 10);

    EXPECT_EQ(count_resource_with(world, ItemID::GOLD_ORB), 0u);
    // Y sí hay roca de relleno, el material más frecuente.
    EXPECT_GT(count_resource_with(world, ItemID::ROCK), 0u);
}

TEST(MineControllerTest, SalaCincoGeneraOro) {
    World world(30, 20);
    MineController mine(5, 42);
    mine.populate(world, 15, 10);

    EXPECT_GT(count_resource_with(world, ItemID::GOLD_ORB), 0u);
}

TEST(MineControllerTest, RegeneraSinAcumularNiBorrarLoDemas) {
    World world(20, 10);
    world.set_entity_at(0, 0, std::make_unique<Bed>(0, 0));

    MineController mine(1, 7);
    mine.populate(world, 5, 5);
    const size_t first = count_kind(world, CellSnapshot::Kind::Resource);
    EXPECT_GT(first, 0u);

    // Un item en el piso: la regeneración lo borra también.
    auto drop = std::make_unique<Product>(ItemID::COAL_ORB, 10, Quality::BASE);
    world.set_entity_at(1, 1, std::make_unique<DroppedItem>(std::move(drop), 1, 1));
    EXPECT_EQ(count_kind(world, CellSnapshot::Kind::Dropped), 1u);

    mine.populate(world, 5, 5);

    // Misma cantidad: no se acumulan generaciones.
    EXPECT_EQ(count_kind(world, CellSnapshot::Kind::Resource), first);
    // Ni minerales ni drops viejos.
    EXPECT_EQ(count_kind(world, CellSnapshot::Kind::Dropped), 0u);
    // La cama sigue viva y nadie se le encima.
    EXPECT_EQ(count_kind(world, CellSnapshot::Kind::Furniture), 1u);
    EXPECT_EQ(world.get_entity_at(0, 0).size(), 1u);
    EXPECT_TRUE(world.get_entity_at(5, 5).empty());
}
