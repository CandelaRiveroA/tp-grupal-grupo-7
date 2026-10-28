#include <cassert>
#include <memory>

#include "game/MockGameClient.h"
#include "game/ResourceAudioCue.h"
#include "server/domain/map/world.h"
#include "server/domain/minerals/gold_rock.h"
#include "server/world_view.h"
int main() {
    World world(4, 4);
    auto player = std::make_unique<Player>(1, 0, 1);
    Player* p = player.get();
    world.add_player(1, std::move(player));
    world.set_player_facing(1, Direction::RIGHT);
    world.set_entity_at(1, 1, std::make_unique<GoldRock>(1, 1));
    WorldView view;
    assert(!view.sync(world, 1).empty());
    assert(world.take_resource_hits().empty());
    p->select_slot(1);
    world.use_tool(1);
    assert(world.take_resource_hits().empty());
    p->select_slot(0);
    game::ResourceAudioCue cue;
    for (int n = 1; n <= 4; ++n) {
        world.use_tool(1);
        auto hits = world.take_resource_hits();
        assert(hits.size() == 1 && hits[0].x == 1 && hits[0].y == 1);
        assert(hits[0].destroyed == (n == 4));
        assert(world.take_resource_hits().empty());
        auto audio = cue.update({{hits[0].x, hits[0].y, hits[0].destroyed}});
        assert(audio.size() == 1);
        assert(audio[0] ==
               (n == 4 ? game::ResourceAudioEvent::Broke : game::ResourceAudioEvent::Hit));
        auto changes = view.sync(world, 1);
        if (n == 2)
            assert(changes.empty());
    }
    world.use_tool(1);
    assert(world.take_resource_hits().empty());
    world.remove_entity_at(1, 1);
    world.use_tool(1);
    assert(world.take_resource_hits().empty());
    assert(cue.update({}).empty());
    assert(cue.update({{1, 1, false}, {1, 1, false}, {2, 2, true}}).size() == 3);
    game::MockGameClient mock(30, 20, 15, 10, 1);
    for (int n = 1; n <= 3; ++n) {
        mock.sendCommand(protocol::PickMessage{Direction::RIGHT});
        mock.update();
        const auto& hits = mock.worldState().resourceHits;
        assert(hits.size() == 1 && hits[0].destroyed == (n == 3));
        assert(mock.worldState().resourceHits.size() == 1);
        mock.update();
        assert(mock.worldState().resourceHits.empty());
    }
}
