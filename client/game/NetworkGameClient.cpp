#include "game/NetworkGameClient.h"

#include <exception>
#include <map>
#include <optional>

#include <sys/socket.h>

#include "world/CreatureCatalog.h"
#include "world/ResourceCatalog.h"

namespace game {

namespace {

// Vocabulario compartido con el servidor (inventory.toml y resources.toml).
std::optional<world::ResourceKind> resourceKindFor(const std::string& contentId) {
    static const std::map<std::string, world::ResourceKind> kinds = {
            {"silver", world::ResourceKind::Silver},
            {"coal", world::ResourceKind::Coal},
            {"lapis", world::ResourceKind::Lapis},
            {"gold", world::ResourceKind::Gold},
    };
    const auto it = kinds.find(contentId);
    if (it == kinds.end()) {
        return std::nullopt;
    }
    return it->second;
}

world::ResourceState resourceStateFor(ResourceDamage damage) {
    static const std::map<ResourceDamage, world::ResourceState> states = {
            {ResourceDamage::Intact, world::ResourceState::Intact},
            {ResourceDamage::Cracked, world::ResourceState::Cracked},
            {ResourceDamage::Broken, world::ResourceState::Broken}};

    const auto it = states.find(damage);
    return it != states.end() ? it->second : world::ResourceState::Intact;
}

}  // namespace

NetworkGameClient::NetworkGameClient(const char* host, const char* port, int toolUseDurationTicks):
        socket_(host, port),
        sender_(socket_),
        receiver_(socket_),
        clientSender_(sender_, commands_),
        clientReceiver_(receiver_, messages_),
        localPlayerId_(receiver_.receive_welcome().player_id),
        toolUseDurationTicks_(toolUseDurationTicks > 0 ? toolUseDurationTicks : 1) {
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

void NetworkGameClient::apply(const ServerMessage& message) {
    if (std::holds_alternative<ServerReset>(message)) {
        // Cada lote describe el estado completo: lo que no vuelve ya no existe.
        // players_ no se borra porque ServerStatus reescribe todas las
        // posiciones y ServerLeave ya borra a quien se fue.
        resources_.clear();
        dropped_.clear();
        inventory_.clear();
        return;
    }
    if (const auto* leave = std::get_if<ServerLeave>(&message)) {
        players_.erase(leave->player_id);
        return;
    }
    if (const auto* status = std::get_if<ServerStatus>(&message)) {
        RemotePlayer& player = players_[status->player_id];
        player.moving = player.cellX != status->x || player.cellY != status->y;
        player.cellX = status->x;
        player.cellY = status->y;
        player.facing = world::viewDirectionFrom(status->direction);
        return;
    }
    if (const auto* hit = std::get_if<ServerResourceHit>(&message)) {
        view_.resourceHits.push_back(ViewResourceHit{hit->x, hit->y, hit->destroyed});
        return;
    }
    if (const auto* resource = std::get_if<ServerResource>(&message)) {
        const std::optional<world::ResourceKind> kind = resourceKindFor(resource->content_id);
        if (kind.has_value()) {
            resources_[{resource->x, resource->y}] = ViewResource{
                    kind.value(), resource->x, resource->y, resourceStateFor(resource->damage)};
        }
        return;
    }
    if (const auto* dropped = std::get_if<ServerDropped>(&message)) {
        dropped_[{dropped->x, dropped->y}] =
                ViewDroppedItem{dropped->content_id, dropped->x, dropped->y};
        return;
    }
    if (const auto* slot = std::get_if<ServerInventorySlot>(&message)) {
        if (slot->count == 0) {
            inventory_.erase(slot->slot);
        } else {
            inventory_[slot->slot] = ViewInventorySlot{slot->slot, slot->content_id, slot->count};
        }
        return;
    }
    if (const auto* obtained = std::get_if<ServerObtained>(&message)) {
        const std::optional<world::ResourceKind> kind = resourceKindFor(obtained->content_id);
        if (kind.has_value()) {
            obtained_.push_back(ViewResourceObtained{kind.value()});
        }
        return;
    }
    if (const auto* used = std::get_if<ServerToolUsed>(&message)) {
        const auto it = players_.find(used->player_id);
        if (it != players_.end()) {
            it->second.toolUseRemaining = toolUseDurationTicks_;
        }
    }
}

void NetworkGameClient::update() {
    view_.resourceHits.clear();
    for (auto& entry: players_) {
        entry.second.moving = false;
    }

    ServerMessage message = ServerLeave{0};
    while (messages_.try_pop(message)) {
        apply(message);
    }

    // La entidad 0 es siempre el jugador local (la vista lo asume).
    view_.entities.clear();
    for (auto& [id, player]: players_) {
        const bool usingTool = player.toolUseRemaining > 0;
        if (usingTool) {
            --player.toolUseRemaining;
        }
        ViewEntity entity{world::CreatureKind::Player,
                          player.cellX,
                          player.cellY,
                          player.facing,
                          player.moving,
                          player.moving ? world::ActionKind::Walk : world::ActionKind::Idle,
                          std::nullopt};
        if (usingTool) {
            entity.action = world::ActionKind::UseTool;
            entity.activeTool = world::ItemKind::Pickaxe;
        }
        if (id == localPlayerId_) {
            view_.entities.insert(view_.entities.begin(), entity);
        } else {
            view_.entities.push_back(entity);
        }
    }

    view_.resources.clear();
    for (const auto& [cell, resource]: resources_) {
        view_.resources.push_back(resource);
    }
    view_.droppedItems.clear();
    for (const auto& [cell, dropped]: dropped_) {
        view_.droppedItems.push_back(dropped);
    }
    view_.inventory.clear();
    for (const auto& [index, slot]: inventory_) {
        view_.inventory.push_back(slot);
    }
    view_.obtained = std::move(obtained_);
    obtained_.clear();
}

const LocalWorldView& NetworkGameClient::worldState() const { return view_; }

}  // namespace game
