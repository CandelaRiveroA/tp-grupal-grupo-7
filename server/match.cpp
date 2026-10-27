#include "match.h"

#include <exception>
#include <memory>
#include <stdexcept>

#define kWidth 20
#define kHeight 15
#define kSpawnX 0
#define kSpawnY 0

Match::Match(Queue<MatchEvent>& events, ServerOutputMonitor& output):
        events(events), output(output), world(kWidth, kHeight) {}

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

void Match::handle(const PlayerLeft& left) {
    const auto it = outputs.find(left.player_id);
    if (it == outputs.end()) {
        return;
    }
    const std::shared_ptr<Queue<ServerMessage>> leaving = it->second;
    outputs.erase(it);
    world.remove_player(left.player_id);
    broadcast(ServerLeave{left.player_id});
    leaving->close();
}

void Match::closeOutputs() {
    for (auto& [id, statuses]: outputs) {
        try {
            statuses->close();
        } catch (const std::runtime_error&) {}
    }
    outputs.clear();
}

void Match::publish(uint8_t player_id) {
    const Player& player = world.get_player(player_id);
    broadcast(ServerStatus(player_id, player.get_x(), player.get_y(), player.get_facing()));
}

void Match::broadcast(const ServerMessage& message) {
    for (const auto& [id, statuses]: outputs) {
        try {
            statuses->push(message);
        } catch (const ClosedQueue&) {
            // Un cliente desconectado no debe detener la partida para los demas.
        }
    }
}

void Match::execute() {
    try {
        while (should_keep_running()) {
            const MatchEvent event = events.pop();
            std::visit([this](const auto& alternative) { handle(alternative); }, event);
        }
    } catch (const ClosedQueue&) {
        closeOutputs();
        throw;
    } catch (const std::exception& error) {
        closeOutputs();
        events.close();
        if (should_keep_running()) {
            output.printError("Error en la partida: ", error.what());
        }
        throw;
    }
}
