#include "common_protocol_sender.h"

#include <limits>
#include <stdexcept>

#include <arpa/inet.h>

ProtocolSender::ProtocolSender(Socket& socket): socket(socket) {}

void ProtocolSender::send_uint8(uint8_t value) {
    if (socket.sendall(&value, sizeof(value)) != sizeof(value)) {
        throw std::runtime_error("No se pudo enviar un entero de 8 bits.");
    }
}

void ProtocolSender::send_uint16(uint16_t value) {
    const uint16_t network_value = htons(value);
    if (socket.sendall(&network_value, sizeof(network_value)) != sizeof(network_value)) {
        throw std::runtime_error("No se pudo enviar un entero de 16 bits.");
    }
}

void ProtocolSender::send_string(const std::string& value) {
    if (value.size() > std::numeric_limits<uint16_t>::max()) {
        throw std::length_error("La cadena del protocolo es demasiado larga.");
    }

    send_uint16(static_cast<uint16_t>(value.size()));
    if (!value.empty() && socket.sendall(value.data(), static_cast<unsigned int>(value.size())) !=
                                  static_cast<int>(value.size())) {
        throw std::runtime_error("No se pudo enviar la cadena del protocolo.");
    }
}

void ProtocolSender::send_appearance(const CharacterAppearance& appearance) {
    send_uint8(appearance.skin);
    send_uint8(appearance.eyes);
    send_uint8(appearance.hair);
    send_uint8(appearance.hairColor);
    send_uint8(appearance.shirt);
    send_uint8(appearance.pants);
    send_uint8(appearance.pantsColor);
    send_uint8(appearance.accessory);
}

void ProtocolSender::validate_direction(Direction direction) {
    const uint8_t raw_direction = static_cast<uint8_t>(direction);
    if (raw_direction > static_cast<uint8_t>(Direction::RIGHT)) {
        throw std::invalid_argument("La direccion del mensaje no es valida.");
    }
}

void ProtocolSender::send_message(const protocol::MoveMessage& message) {
    validate_direction(message.direction);

    send_uint8(static_cast<uint8_t>(protocol::ClientMessageType::Move));
    send_uint8(static_cast<uint8_t>(message.direction));
}

void ProtocolSender::send_message(const protocol::PickMessage& message) {
    validate_direction(message.direction);

    send_uint8(static_cast<uint8_t>(protocol::ClientMessageType::Pick));
    send_uint8(static_cast<uint8_t>(message.direction));
    send_uint8(message.slot);
}

void ProtocolSender::send_message(const protocol::TakeMessage& message) {
    validate_direction(message.direction);

    send_uint8(static_cast<uint8_t>(protocol::ClientMessageType::Take));
    send_uint8(static_cast<uint8_t>(message.direction));
    send_uint8(message.slot);
}

void ProtocolSender::send_message(const protocol::SleepMessage& message) {
    send_uint8(static_cast<uint8_t>(protocol::ClientMessageType::Sleep));
    send_uint8(message.accept ? 1 : 0);
}

void ProtocolSender::send_message(const protocol::WakeMessage& /*message*/) {
    send_uint8(static_cast<uint8_t>(protocol::ClientMessageType::Wake));
}

void ProtocolSender::send_message(const protocol::ChatMessage& message) {
    send_uint8(static_cast<uint8_t>(protocol::ClientMessageType::Chat));
    send_string(message.text);
}

void ProtocolSender::send_message(const protocol::ToggleWalkingMessage& /*message*/) {
    send_uint8(static_cast<uint8_t>(protocol::ClientMessageType::ToggleWalking));
}

void ProtocolSender::send_client_message(const protocol::ClientMessage& message) {
    std::visit([this](const auto& alternative) { send_message(alternative); }, message);
}

void ProtocolSender::send_message(const ServerStatus& status) {
    validate_direction(status.direction);

    send_uint8(static_cast<uint8_t>(ServerMessageType::Status));
    send_uint8(status.player_id);
    send_uint16(status.x);
    send_uint16(status.y);
    send_uint8(static_cast<uint8_t>(status.direction));
    send_appearance(status.appearance);
    send_uint16(status.health);
    send_uint16(status.max_health);
    send_uint16(status.energy);
    send_uint16(status.max_energy);
    send_uint8(status.fainted);
}
void ProtocolSender::send_message(const ServerLeave& leave) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Leave));
    send_uint8(leave.player_id);
}

void ProtocolSender::send_message(const ServerResource& resource) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Resource));
    send_uint16(resource.x);
    send_uint16(resource.y);
    send_string(resource.content_id);
    send_uint8(static_cast<uint8_t>(resource.damage));
}

void ProtocolSender::send_message(const ServerDropped& dropped) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Dropped));
    send_uint16(dropped.x);
    send_uint16(dropped.y);
    send_string(dropped.content_id);
    send_uint8(dropped.interactive ? 1 : 0);
}

void ProtocolSender::send_message(const ServerInventorySlot& slot) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::InventorySlot));
    send_uint8(slot.slot);
    send_uint8(slot.count);
    send_string(slot.content_id);
    send_uint8(slot.tradable ? 1 : 0);
}

void ProtocolSender::send_message(const ServerObtained& obtained) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Obtained));
    send_string(obtained.content_id);
}

void ProtocolSender::send_message(const ServerToolUsed& used) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::ToolUsed));
    send_uint8(used.player_id);
    send_string(used.content_id);
}

void ProtocolSender::send_message(const ServerResourceHit& hit) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::ResourceHit));
    send_uint16(hit.x);
    send_uint16(hit.y);
    send_uint8(hit.destroyed ? 1 : 0);
}

void ProtocolSender::send_message(const ServerReset& /*reset*/) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Reset));
}

void ProtocolSender::send_message(const ServerSyncEnd& /*end*/) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::SyncEnd));
}

void ProtocolSender::send_message(const ServerAnimal& animal) {
    validate_direction(animal.direction);

    send_uint8(static_cast<uint8_t>(ServerMessageType::Animal));
    send_uint8(animal.id);
    send_string(animal.kind);
    send_uint16(animal.x);
    send_uint16(animal.y);
    send_uint8(static_cast<uint8_t>(animal.direction));
    send_uint8(animal.ready_to_milk ? 1 : 0);
}

void ProtocolSender::send_message(const ServerEnemy& enemy) {
    validate_direction(enemy.direction);

    send_uint8(static_cast<uint8_t>(ServerMessageType::Enemy));
    send_uint16(enemy.id);
    send_string(enemy.kind);
    send_uint16(enemy.x);
    send_uint16(enemy.y);
    send_uint8(static_cast<uint8_t>(enemy.direction));
    send_uint8(enemy.health);
    send_uint8(static_cast<uint8_t>(enemy.action));
}

void ProtocolSender::send_message(const ServerProjectile& projectile) {
    validate_direction(projectile.direction);

    send_uint8(static_cast<uint8_t>(ServerMessageType::Projectile));
    send_uint16(projectile.id);
    send_string(projectile.kind);
    send_uint16(projectile.x);
    send_uint16(projectile.y);
    send_uint8(static_cast<uint8_t>(projectile.direction));
}

void ProtocolSender::send_message(const ServerSleep& sleep) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Sleep));
    send_uint8(sleep.sleeping);
    send_uint8(sleep.total);
}

void ProtocolSender::send_message(const ServerFurniture& furniture) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Furniture));
    send_uint16(furniture.x);
    send_uint16(furniture.y);
    send_string(furniture.content_id);
}

void ProtocolSender::send_message(const ServerDayEnd& /*day*/) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::DayEnd));
}

void ProtocolSender::send_message(const ServerDay& /*day*/) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Day));
}

void ProtocolSender::send_message(const ServerSleepingPlayers& sleepers) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::SleepingPlayers));
    send_uint8(static_cast<uint8_t>(sleepers.players.size()));
    for (const uint8_t id: sleepers.players) {
        send_uint8(id);
    }
}

void ProtocolSender::send_message(const ServerClock& clock) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Clock));
    send_uint8(clock.day);
    send_uint16(clock.minutes);
}

void ProtocolSender::send_message(const ServerCrop& crop) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Crop));
    send_uint16(crop.x);
    send_uint16(crop.y);
    send_string(crop.crop_id);
    send_uint8(crop.stage);
    send_uint8(crop.needs_water ? 1 : 0);
    send_uint8(crop.interactive ? 1 : 0);
    send_uint8(crop.quality);
}

void ProtocolSender::send_message(const ServerGroundEffect& ground) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::GroundEffect));
    send_uint16(ground.x);
    send_uint16(ground.y);
    send_uint8(static_cast<uint8_t>(ground.effect));
}

void ProtocolSender::send_message(const ServerChat& chat) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Chat));
    send_uint8(chat.player_id);
    send_string(chat.characterName);
    send_string(chat.text);
    send_uint8(chat.system ? 1 : 0);
}

void ProtocolSender::send_message(const ServerGold& gold) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Gold));
    send_uint16(gold.gold);
}

void ProtocolSender::send_message(const ServerNpcPlacement& placement) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::NpcPlacement));
    send_string(placement.id);
    send_uint16(placement.x);
    send_uint16(placement.y);
}

void ProtocolSender::send_message(const ServerMapChange& change) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::MapChange));
    send_string(change.mapId);
}

void ProtocolSender::send_message(const ServerMarketOffer& offer) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::MarketOffer));
    send_string(offer.merchant_id);
    send_uint16(static_cast<uint16_t>(offer.entries.size()));
    for (const ServerMarketEntry& entry: offer.entries) {
        send_string(entry.content_id);
        send_string(entry.label);
        send_uint8(static_cast<uint8_t>(entry.side));
        send_uint8(entry.has_price ? 1 : 0);
        send_uint16(entry.price);
    }
}

void ProtocolSender::send_server_message(const ServerMessage& message) {
    std::visit([this](const auto& alternative) { send_message(alternative); }, message);
}

void ProtocolSender::send_welcome(const ServerWelcome& welcome) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Welcome));
    send_uint8(welcome.player_id);
    send_string(welcome.worldName);
}

void ProtocolSender::send_match_request(const MatchRequest& request) {
    send_uint8(static_cast<uint8_t>(request.type));
    send_string(request.codeMatch);
    send_string(request.nameMatch);
    send_string(request.worldName);
    send_string(request.username);
    send_string(request.characterName);
    send_appearance(request.appearance);
}

void ProtocolSender::send_lobby_status(LobbyStatus status) {
    send_uint8(static_cast<uint8_t>(status));
}

void ProtocolSender::send_world_list_request() {
    send_uint8(static_cast<uint8_t>(MatchRequestType::ListWorlds));
}

void ProtocolSender::send_world_list(const std::vector<std::string>& worlds) {
    if (worlds.size() > std::numeric_limits<uint16_t>::max()) {
        throw std::length_error("La lista de mundos del protocolo es demasiado larga.");
    }
    send_uint16(static_cast<uint16_t>(worlds.size()));
    for (const std::string& world : worlds) {
        send_string(world);
    }
}

void ProtocolSender::send_message(const protocol::GiftResponseMessage& message) {
    send_uint8(static_cast<uint8_t>(protocol::ClientMessageType::GiftResponse));
    send_uint8(message.sender_id);
    send_uint8(message.accept ? 1 : 0);
}

void ProtocolSender::send_message(const ServerGiftRequest& request) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::GiftRequest));
    send_uint8(request.sender_id);
    send_string(request.characterName);
    send_string(request.content_id);
}

void ProtocolSender::send_message(const ServerGiftAnswered& /*answered*/) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::GiftAnswered));
}

void ProtocolSender::send_message(const ServerSetCompleted& completed) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::SetCompleted));
    send_uint8(completed.set_id);
}

void ProtocolSender::send_message(const ServerVictory& /*victory*/) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Victory));
}

void ProtocolSender::send_message(const protocol::SelectSlotMessage& message) {
    send_uint8(static_cast<uint8_t>(protocol::ClientMessageType::SelectSlot));
    send_uint8(message.slot);
}

void ProtocolSender::send_message(const ServerPlayerHeld& message) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::PlayerHeld));
    send_uint8(message.player_id);
    send_string(message.content_id);
}

void ProtocolSender::send_message(const ServerGiftAccepted& message) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::GiftAccepted));
    send_uint8(message.receiver_id);
}
