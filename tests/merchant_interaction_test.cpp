#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "../server/domain/map/world.h"
#include "../server/domain/npc/merchant.h"
#include "../server/world_view.h"
#include "game/NpcPlacementResolver.h"

namespace {
worldmodel::World merchantWorld(worldmodel::MarkerKind kind) {
    worldmodel::World model;
    worldmodel::Map map;
    map.id = "general_store";
    map.kind = worldmodel::MapKind::GeneralStore;
    map.width = map.height = 10;
    map.baseCollision.assign(100, false);
    map.collision.assign(100, false);
    map.markers.push_back({kind, {4, 4}});
    model.maps[map.id] = map;
    return model;
}

world::NpcDef pierre() {
    world::NpcDef npc;
    npc.id = "pierre";
    npc.room = "general_store";
    npc.text = "Hola";
    npc.body.at = {4, 4};
    npc.body.span = {1, 2};
    npc.body.cells = {{0, 0}, {0, 1}};
    return npc;
}
}

TEST(MerchantInteractionTest, LosCuatroMercaderesBloqueanLosPiesYNoLaCabeza) {
    for (const auto kind: {worldmodel::MarkerKind::MerchantPierre,
                           worldmodel::MarkerKind::MerchantMarnie,
                           worldmodel::MarkerKind::MerchantClint,
                           worldmodel::MarkerKind::MerchantRobin}) {
        World world(merchantWorld(kind));
        const Scene& scene = world.get_scene("general_store");
        EXPECT_TRUE(scene.can_walk_to(4, 4));
        EXPECT_FALSE(scene.can_walk_to(4, 5));
        EXPECT_TRUE(scene.can_walk_to(4, 6));
        const auto placements = scene.get_merchant_placements();
        ASSERT_EQ(placements.size(), 1u);
        EXPECT_EQ(placements[0].x, 4);
        EXPECT_EQ(placements[0].y, 5);
    }
}

TEST(MerchantInteractionTest, RadioDosIncluyeLas24VecinasYExcluyeElTercerAnillo) {
    Scene scene(12, 12);
    scene.place_merchant(std::make_unique<Merchant>(5, 5, "Pierre", "pierre"));
    int neighbors = 0;
    for (int dy = -3; dy <= 3; ++dy) {
        for (int dx = -3; dx <= 3; ++dx) {
            if (dx == 0 && dy == 0) continue;
            const bool inside = dx >= -2 && dx <= 2 && dy >= -2 && dy <= 2;
            const auto* merchant = scene.get_nearest_merchant(
                    static_cast<uint16_t>(5 + dx), static_cast<uint16_t>(5 + dy));
            EXPECT_EQ(merchant != nullptr, inside);
            if (merchant) ++neighbors;
        }
    }
    EXPECT_EQ(neighbors, 24);
}

TEST(MerchantInteractionTest, MercaderSeleccionadoSeRevalidaAlAlejarse) {
    World world(merchantWorld(worldmodel::MarkerKind::MerchantPierre));
    world.add_player(1, std::make_unique<Player>(1, 5, 5, "general_store"));
    world.take_item(1);  // selecciona Pierre
    world.set_player_position(1, 6, 7);  // esquina a distancia 2 de sus pies
    ASSERT_NE(world.get_merchant_for_command(1), nullptr);
    EXPECT_TRUE(world.process_merchant_command(1, "/consultar_catalogo").empty());
    world.set_player_position(1, 7, 5);
    EXPECT_EQ(world.get_merchant_for_command(1), nullptr);
    EXPECT_NE(world.process_merchant_command(1, "/consultar_catalogo").find(
                      "No hay ningun comerciante cerca"), std::string::npos);
}

TEST(MerchantInteractionTest, RedDePiesAMapaConservaElDibujoYLaCajaDelClick) {
    const world::NpcDef npc = pierre();
    game::LocalWorldView view;
    view.npcPlacements.push_back({"pierre", 4, 5});
    const game::NpcPlacementResolver resolver;
    world::SceneDescription scene{};
    const world::NpcCells cells = resolver.cellsFrom(view, scene);
    const auto top = npc.cellIn(cells);
    EXPECT_EQ(top.column, 4);
    EXPECT_EQ(top.row, 4);
    const auto box = npc.body.box(16, top);
    EXPECT_EQ(box.x, 64);
    EXPECT_EQ(box.y, 64);
    EXPECT_EQ(box.height, 32);
    const auto pieces = npc.body.pieces(16, top, 0);
    ASSERT_EQ(pieces.size(), 2u);
    EXPECT_EQ(pieces.back().dest.y / 16, 5);
}

TEST(MerchantInteractionTest, PopupYCursorUsanElRadioDeLosPies) {
    const auto npc = pierre();
    game::LocalWorldView view;
    view.entities.push_back(game::ViewEntity{});
    view.npcPlacements.push_back({"pierre", 4, 5});
    const game::NpcPlacementResolver resolver;
    const world::SceneDescription scene{};
    const auto cells = resolver.cellsFrom(view, scene);
    for (int y = 1; y <= 9; ++y) {
        for (int x = 0; x <= 8; ++x) {
            view.entities[0].cellX = x;
            view.entities[0].cellY = y;
            const bool inRange = x >= 2 && x <= 6 && y >= 3 && y <= 7;
            EXPECT_EQ(resolver.canInteractWith(npc, view, cells), inRange);
        }
    }
    view.entities.clear();
    EXPECT_FALSE(resolver.canInteractWith(npc, view, cells));
}

TEST(MerchantInteractionTest, MapaSinMensajeDeRedUsaElMismoPuntoDeInteraccion) {
    const auto npc = pierre();
    game::LocalWorldView view;
    view.entities.push_back(game::ViewEntity{});
    view.entities[0].cellX = 6;
    view.entities[0].cellY = 7;
    const game::NpcPlacementResolver resolver;
    world::SceneDescription scene{};
    scene.npcCells["pierre"] = {4, 4};
    const auto cells = resolver.cellsFrom(view, scene);
    EXPECT_TRUE(resolver.canInteractWith(npc, view, cells));
    EXPECT_TRUE(resolver.canInteractWith(npc, view, {})); // fallback de npcs.toml
    view.entities[0].cellY = 8;
    EXPECT_FALSE(resolver.canInteractWith(npc, view, cells));
}

TEST(MerchantInteractionTest, CofreSinPopupConservaSuPosicionDeMapa) {
    world::NpcDef chest;
    chest.id = "cofre";
    chest.body.at = {19, 12};
    game::LocalWorldView view;
    const game::NpcPlacementResolver resolver;
    const world::SceneDescription scene{};
    const auto cells = resolver.cellsFrom(view, scene);
    EXPECT_EQ(chest.cellIn(cells).row, 12);
    EXPECT_TRUE(resolver.canInteractWith(chest, view, cells));
}
