#include <memory>

#include "game/MockGameClient.h"
#include "game/ResourceAudioCue.h"
#include "gtest/gtest.h"
#include "server/domain/map/world.h"
#include "server/domain/minerals/gold_rock.h"
#include "server/world_view.h"

// Pipeline completo del golpe: dominio (world.use_tool -> ResourceHit),
// vista (WorldView::sync -> ServerMessage) y audio (ResourceAudioCue),
// mas la version cliente del mismo flujo via MockGameClient.
//
// NOTA: este archivo integra el ex-"main" de humo como TEST de gtest.
// Antes definia int main() y compilaba dentro de taller_tests, que linkea
// GTest::gtest_main; el main del objeto ganaba al del archive, RUN_ALL_TESTS
// no corria nunca y el binario devolvia 0 sin ejecutar los tests (verde
// vacio). Ver commit de este fix para el detalle.
TEST(ResourceHitSmoke, PipelineCompletoDelGolpeLocal) {
    World world(4, 4);
    auto player = std::make_unique<Player>(1, 0, 1);
    Player* p = player.get();
    world.add_player(1, std::move(player));
    world.set_player_facing(1, Direction::RIGHT);
    world.set_entity_at(1, 1, std::make_unique<GoldRock>(1, 1));
    WorldView view;

    // Primer sync: el mundo completo.
    EXPECT_FALSE(view.sync(world, 1).empty());
    EXPECT_TRUE(world.take_resource_hits().empty());

    // Slot 1 (pala): no es herramienta de minado, no hay golpes.
    p->select_slot(1);
    world.use_tool(1);
    EXPECT_TRUE(world.take_resource_hits().empty());

    // Slot 0 (pico): 4 golpes agotan la roca de oro.
    p->select_slot(0);
    game::ResourceAudioCue cue;
    for (int n = 1; n <= 4; ++n) {
        world.use_tool(1);
        const std::vector<ResourceHit> hits = world.take_resource_hits();
        ASSERT_EQ(hits.size(), 1u);
        EXPECT_EQ(hits[0].x, 1);
        EXPECT_EQ(hits[0].y, 1);
        EXPECT_EQ(hits[0].destroyed, n == 4);
        // La cola del dominio se vacia por consulta, no se duplica.
        EXPECT_TRUE(world.take_resource_hits().empty());

        const std::vector<game::ResourceAudioEvent> audio =
                cue.update({{hits[0].x, hits[0].y, hits[0].destroyed}});
        ASSERT_EQ(audio.size(), 1u);
        EXPECT_EQ(audio[0],
                  (n == 4 ? game::ResourceAudioEvent::Broke
                          : game::ResourceAudioEvent::Hit));

        const std::vector<ServerMessage> changes = view.sync(world, 1);
        if (n == 2) {
            // Cracked -> Cracked: impacto sonoro sin cambio visual.
            EXPECT_TRUE(changes.empty());
        }
    }

    // Roca agotada: ni golpes pedidos de mas ni sobre celda vacia.
    world.use_tool(1);
    EXPECT_TRUE(world.take_resource_hits().empty());
    world.remove_entity_at(1, 1);
    world.use_tool(1);
    EXPECT_TRUE(world.take_resource_hits().empty());
    EXPECT_TRUE(cue.update({}).empty());
    EXPECT_EQ(cue.update({{1, 1, false}, {1, 1, false}, {2, 2, true}}).size(),
              3u);

    // El mock reproduce lo mismo de su lado: golpe confirmado al terminar
    // la ventana, y resourceHits dura un tick en la vista.
    game::MockGameClient mock(30, 20, 15, 10, 1);
    for (int n = 1; n <= 3; ++n) {
        mock.sendCommand(protocol::PickMessage{Direction::RIGHT});
        mock.update();
        ASSERT_EQ(mock.worldState().resourceHits.size(), 1u);
        EXPECT_EQ(mock.worldState().resourceHits[0].destroyed, n == 3);
        mock.update();
        EXPECT_TRUE(mock.worldState().resourceHits.empty());
    }
}
