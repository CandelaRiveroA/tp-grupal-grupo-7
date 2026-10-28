#include "game/NetworkGameClient.h"

#include <exception>

#include <sys/socket.h>

#include "world/CreatureCatalog.h"

namespace game {

NetworkGameClient::NetworkGameClient(const char* host, const char* port):
        socket_(host, port),
        sender_(socket_),
        receiver_(socket_),
        clientSender_(sender_, commands_),
        clientReceiver_(receiver_, messages_),
        localPlayerId_(receiver_.receive_welcome().player_id) {
    rebuildInventory();
    clientSender_.start();
    clientReceiver_.start();
}

NetworkGameClient::~NetworkGameClient() {
    commands_.close();
    try {
        socket_.shutdown(SHUT_RDWR);
    } catch (const std::exception&) {}
    clientSender_.join();
    clientReceiver_.join();
}

void NetworkGameClient::sendCommand(const protocol::ClientMessage& message) {
    commands_.push(message);
}

const LocalWorldView& NetworkGameClient::worldState() const {
    for (auto& entry: players_) {
        entry.second.moving = false;
    }

    ServerMessage message = ServerLeave{0};
    while (messages_.try_pop(message)) {
        if (const auto* leave = std::get_if<ServerLeave>(&message)) {
            players_.erase(leave->player_id);
            continue;
        }
        const ServerStatus& status = std::get<ServerStatus>(message);
        RemotePlayer& player = players_[status.player_id];
        player.moving = player.cellX != status.x || player.cellY != status.y;
        player.cellX = status.x;
        player.cellY = status.y;
        player.facing = world::viewDirectionFrom(status.direction);
    }

    view_.entities.clear();
    const auto local = players_.find(localPlayerId_);
    if (local != players_.end()) {
        view_.entities.push_back(ViewEntity{
                world::CreatureKind::Player, local->second.cellX, local->second.cellY,
                local->second.facing, local->second.moving,
                local->second.moving ? world::ActionKind::Walk : world::ActionKind::Idle,
                std::nullopt});
    }
    for (const auto& [id, player]: players_) {
        if (id == localPlayerId_) {
            continue;
        }
        view_.entities.push_back(ViewEntity{
                world::CreatureKind::Player, player.cellX, player.cellY, player.facing,
                player.moving, player.moving ? world::ActionKind::Walk : world::ActionKind::Idle,
                std::nullopt});
    }

    return view_;
}

void NetworkGameClient::rebuildInventory() const {
    view_.inventory.clear();
    view_.inventory.push_back({0, "shovel", 1});
    view_.inventory.push_back({1, "pickaxe", 1});
    view_.inventory.push_back({2, "sword", 1});
    view_.inventory.push_back({3, "watering_can", 1});
}

}  // namespace game
