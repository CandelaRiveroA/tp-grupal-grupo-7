#include "match.h"

#include <memory>

#define kWidth 20
#define kHeight 15
#define kSpawnX 0
#define kSpawnY 0

Match::Match(Queue<MatchEvent>& events): events(events), world(kWidth, kHeight) {}

void Match::handle(const PlayerJoined& joined) {
    outputs[joined.player_id] = joined.statuses;
    world.add_player(joined.player_id,
                     std::make_unique<Player>(joined.player_id, kSpawnX, kSpawnY));
    publish(joined.player_id);
}

void Match::handle(const PlayerMessage& event) {
    std::visit([this, &event](const auto& message) { apply(event.player_id, message); },
               event.message);
}

void Match::apply(uint8_t player_id, const protocol::MoveMessage& message) {
    world.move_player(player_id, message.direction);
    publish(player_id);
}

void Match::publish(uint8_t player_id) {
    const Player& player = world.get_player(player_id);
    const ServerStatus status(player_id, player.get_x(), player.get_y(), player.get_facing());

    for (const auto& [id, statuses]: outputs) {
        try {
            statuses->push(status);
        } catch (const ClosedQueue&) {
            // Un cliente desconectado no debe detener la partida para los demas.
        }
    }
}

void Match::execute() {
    while (should_keep_running()) {
        const MatchEvent event = events.pop();
        std::visit([this](const auto& alternative) { handle(alternative); }, event);
    }
}
