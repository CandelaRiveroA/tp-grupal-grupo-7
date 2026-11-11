#include "game/NetworkGameClient.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <map>
#include <optional>
#include <variant>

#include <sys/socket.h>

#include "game/GameConstants.h"
#include "world/ContentIds.h"
#include "world/CreatureCatalog.h"
#include "world/ResourceCatalog.h"

namespace game {

namespace {

// Vocabulario compartido con el servidor (characters.toml).
std::optional<world::CreatureKind> creatureKindFor(const std::string& kind) {
    static const std::map<std::string, world::CreatureKind> kinds = {
            {"chicken", world::CreatureKind::Chicken},
            {"cow", world::CreatureKind::Cow},
    };
    const auto it = kinds.find(kind);
    if (it == kinds.end()) {
        return std::nullopt;
    }
    return it->second;
}

// Contenido equipado (inventory.toml) -> herramienta. Sin fila = nullopt (manos o producto).
std::optional<world::ItemKind> toolKindFor(const std::string& contentId) {
    static const world::ContentIds contentIds;
    return contentIds.itemKindFor(contentId);
}

}  // namespace

NetworkGameClient::NetworkGameClient(Socket&& socket, int toolUseDurationTicks,
                                     const world::CharacterCatalog& characters):
        socket_(std::move(socket)),
        sender_(socket_),
        receiver_(socket_),
        clientSender_(sender_, commands_),
        clientReceiver_(receiver_, messages_),
        localPlayerId_(receiver_.receive_welcome().player_id),
        toolUseDurationTicks_(toolUseDurationTicks > 0 ? toolUseDurationTicks : 1),
        characters_(&characters),
        resourceStates_{{ResourceDamage::Intact, world::ResourceState::Intact},
                        {ResourceDamage::Cracked, world::ResourceState::Cracked},
                        {ResourceDamage::Broken, world::ResourceState::Broken}} {
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

world::ResourceState NetworkGameClient::resourceStateFor(ResourceDamage damage) const {
    const auto it = resourceStates_.find(damage);
    return it != resourceStates_.end() ? it->second : world::ResourceState::Intact;
}

void NetworkGameClient::sendCommand(const protocol::ClientMessage& message) {
    if (std::holds_alternative<protocol::TakeMessage>(message)) {
        // Orden del mock: primero los animales (ordeño), después el piso (recogida).
        startMilking();
        startPickup();
    }
    commands_.push(message);
}

void NetworkGameClient::startMilking() {
    const auto player = players_.find(localPlayerId_);
    if (session_.active() || player == players_.end()) {
        return;
    }
    const RemotePlayer& self = player->second;
    for (const auto& [id, animal]: animals_) {
        // Lista para ordeñar = con indicación (la fuente la pone solo a las listas).
        if (!animal.indication.has_value() || std::abs(animal.cellX - self.cellX) > 1 ||
            std::abs(animal.cellY - self.cellY) > 1) {
            continue;
        }
        InteractionTarget target;
        target.action = world::ActionKind::Milk;
        target.cycles = kDefaultMilkCycles;
        target.targetId = id;
        target.cellX = animal.cellX;
        target.cellY = animal.cellY;
        session_.start(target, self.cellX, self.cellY,
                       characters_->dataFor(world::CreatureKind::Player));
        return;
    }
}

void NetworkGameClient::startPickup() {
    if (session_.active()) {
        return;  // ya hay una interacción abierta (el ordeño va primero)
    }
    const auto player = players_.find(localPlayerId_);
    if (player == players_.end()) {
        return;
    }
    const RemotePlayer& self = player->second;
    // Abre la animación de recogida sobre el contenido de una celda.
    const auto startAt = [&](const std::string& contentId, const int cellX, const int cellY,
                             const int targetId) {
        InteractionTarget target;
        target.action = world::ActionKind::Pickup;
        target.cycles = kPickupCycles;
        // La recompensa la entrega el servidor (ServerObtained + el slot), por
        // eso esta sesión no llama mergeRewards: acá solo se anima.
        target.reward = contentId;
        target.heldContent = contentId;
        target.targetId = targetId;
        target.cellX = cellX;
        target.cellY = cellY;
        session_.start(target, self.cellX, self.cellY,
                       characters_->dataFor(world::CreatureKind::Player));
    };
    // Mismo orden que World::take_item: las 8 vecinas fila por fila y, en cada
    // una, primero el ítem del piso y después el cultivo listo.
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            const int cellX = self.cellX + dx;
            const int cellY = self.cellY + dy;
            for (std::size_t i = 0; i < dropped_.size(); ++i) {
                const ViewDroppedItem& item = dropped_[i];
                if (item.interactive && item.cellX == cellX && item.cellY == cellY) {
                    startAt(item.contentId, cellX, cellY, static_cast<int>(i));
                    return;
                }
            }
            for (const ViewCrop& crop: crops_) {
                if (crop.interactive && crop.cellX == cellX && crop.cellY == cellY) {
                    startAt(crop.cropId, cellX, cellY, -1);  // los cultivos no usan targetId
                    return;
                }
            }
        }
    }
}

void NetworkGameClient::apply(const ServerMessage& message) {
    // El lote Reset…SyncEnd se aplica entero de una: el cierre recién ahí
    // limpia y rellena, así nunca se dibuja un estado a medias.
    if (std::holds_alternative<ServerReset>(message)) {
        batch_.clear();
        batch_.push_back(message);
        return;
    }
    if (std::holds_alternative<ServerSyncEnd>(message)) {
        for (const ServerMessage& buffered: batch_) {
            applyNow(buffered);
        }
        batch_.clear();
        return;
    }
    if (!batch_.empty()) {
        batch_.push_back(message);
        return;
    }
    applyNow(message);
}

void NetworkGameClient::applyNow(const ServerMessage& message) {
    if (std::holds_alternative<ServerReset>(message)) {
        // Cada lote describe el estado completo: lo que no vuelve ya no existe.
        // players_ no se borra porque ServerStatus reescribe todas las
        // posiciones y ServerLeave ya borra a quien se fue.
        resources_.clear();
        dropped_.clear();
        crops_.clear();
        groundEffects_.clear();
        inventory_.clear();
        animals_.clear();
        return;
    }
    if (const auto* crop = std::get_if<ServerCrop>(&message)) {
        ViewCrop view{crop->x, crop->y, crop->crop_id, crop->stage};
        if (crop->needs_water) {
            view.indication = world::Indication::ReadyToWater;
        }
        view.interactive = crop->interactive;
        // Calidad 1..5 (BASE..PERFECT) del decoder -> el orden de
        // world::CropQuality (0..4); es el mismo que qualities de crops.toml.
        view.quality = static_cast<world::CropQuality>(crop->quality - 1);
        crops_.push_back(view);
        return;
    }
    if (const auto* ground = std::get_if<ServerGroundEffect>(&message)) {
        groundEffects_.push_back(ViewGroundEffect{
                ground->x, ground->y,
                ground->effect == GroundEffect::Watered ? world::GroundConsequence::Watered
                                                        : world::GroundConsequence::Dug});
        return;
    }
    if (const auto* animal = std::get_if<ServerAnimal>(&message)) {
        const std::optional<world::CreatureKind> kind = creatureKindFor(animal->kind);
        if (kind.has_value()) {
            ViewEntity entity{kind.value(), animal->x,
                              animal->y,    world::viewDirectionFrom(animal->direction),
                              false,        world::ActionKind::Idle,
                              std::nullopt};
            entity.id = animal->id;
            entity.interactive = true;
            if (animal->ready_to_milk) {
                entity.indication = world::Indication::ReadyToMilk;
            }
            animals_[animal->id] = entity;
        }
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
        player.appearance = status->appearance;
        return;
    }
    if (const auto* sleep = std::get_if<ServerSleep>(&message)) {
        view_.sleep = ViewSleep{sleep->sleeping, sleep->total};
        return;
    }
    if (std::holds_alternative<ServerDayEnd>(message)) {
        view_.dayEnded = true;
        return;
    }
    if (std::holds_alternative<ServerDay>(message)) {
        view_.dayChanged = true;
        return;
    }
    if (const auto* sleepers = std::get_if<ServerSleepingPlayers>(&message)) {
        sleepers_ = sleepers->players;
        return;
    }
    if (const auto* clock = std::get_if<ServerClock>(&message)) {
        view_.dayNumber = clock->day;
        view_.clockMinutes = clock->minutes;
        return;
    }
    if (const auto* hit = std::get_if<ServerResourceHit>(&message)) {
        view_.resourceHits.push_back(ViewResourceHit{hit->x, hit->y, hit->destroyed});
        return;
    }
    if (const auto* resource = std::get_if<ServerResource>(&message)) {
        // El contentId viaja tal cual: el vocabulario es abierto y lo
        // resuelve resources.toml en el draw (un id sin sección no se
        // dibuja, igual que un ítem sin visual en ObjectPicker).
        resources_[{resource->x, resource->y}] = ViewResource{
                resource->content_id, resource->x, resource->y,
                resourceStateFor(resource->damage)};
        return;
    }
    if (const auto* dropped = std::get_if<ServerDropped>(&message)) {
        const ServerDropped& drop = *dropped;
        dropped_.push_back(ViewDroppedItem{drop.content_id, drop.x, drop.y, drop.interactive});
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
        // El evento de obtención viaja por content_id (vocabulario
        // genérico): el view lo refleja tal cual.
        obtained_.push_back(ViewObtained{obtained->content_id});
        return;
    }
    if (const auto* used = std::get_if<ServerToolUsed>(&message)) {
        const auto it = players_.find(used->player_id);
        if (it != players_.end()) {
            it->second.toolUseRemaining = toolUseDurationTicks_;
            it->second.activeTool = toolKindFor(used->content_id);
        }
    }
}

void NetworkGameClient::update(int) {
    view_.resourceHits.clear();
    view_.dayEnded = false;
    view_.dayChanged = false;
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
        entity.sleeping = std::find(sleepers_.begin(), sleepers_.end(), id) != sleepers_.end();
        entity.appearance = player.appearance;
        if (usingTool) {
            entity.action = world::ActionKind::UseTool;
            entity.activeTool = player.activeTool;
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
    view_.droppedItems = dropped_;
    view_.plantedCrops = crops_;
    view_.groundEffects = groundEffects_;
    for (const auto& entry: animals_) {
        view_.entities.push_back(entry.second);
    }
    session_.advance();
    if (!view_.entities.empty()) {
        session_.mergePlayer(view_.entities[0]);
    }
    view_.inventory.clear();
    for (const auto& [index, slot]: inventory_) {
        view_.inventory.push_back(slot);
    }
    view_.obtained = std::move(obtained_);
    obtained_.clear();
}

const LocalWorldView& NetworkGameClient::worldState() const { return view_; }

bool NetworkGameClient::connected() const {
    return !clientSender_.failed() && !clientReceiver_.failed();
}

}  // namespace game
