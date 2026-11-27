#ifndef COMMON_PROTOCOL_SENDER_H
#define COMMON_PROTOCOL_SENDER_H

#include <cstdint>
#include <string>
#include <vector>

#include "common_command.h"
#include "common_match_request.h"
#include "common_server_status.h"
#include "common_socket_io.h"

class ProtocolSender {
private:
    SocketIO& socket;

    void send_uint8(uint8_t value);
    void send_uint16(uint16_t value);
    // cppcheck-suppress unusedPrivateFunction
    void send_string(const std::string& value);
    // Los 8 indices de la personalizacion, siempre en el mismo orden.
    void send_appearance(const CharacterAppearance& appearance);

    static void validate_direction(Direction direction);

    void send_message(const protocol::MoveMessage& message);
    void send_message(const protocol::PickMessage& message);
    void send_message(const protocol::TakeMessage& message);
    void send_message(const protocol::SleepMessage& message);
    void send_message(const protocol::WakeMessage& message);
    void send_message(const protocol::ChatMessage& message);
    void send_message(const protocol::ToggleWalkingMessage& message);
    void send_message(const protocol::SelectSlotMessage& message);
    void send_message(const ServerPlayerHeld& message);
    void send_message(const ServerGiftAccepted& message);
    void send_message(const ServerPlayerName& message);
    void send_message(const ServerInventoryFull& message);
    void send_message(const ServerStatus& status);
    void send_message(const ServerLeave& leave);
    void send_message(const ServerResource& resource);
    void send_message(const ServerDropped& dropped);
    void send_message(const ServerInventorySlot& slot);
    void send_message(const ServerObtained& obtained);
    void send_message(const ServerToolUsed& used);
    void send_message(const ServerResourceHit& hit);
    void send_message(const ServerReset& reset);
    void send_message(const ServerSyncEnd& end);
    void send_message(const ServerAnimal& animal);
    void send_message(const ServerEnemy& enemy);
    void send_message(const ServerProjectile& projectile);
    void send_message(const ServerSleep& sleep);
    void send_message(const ServerFurniture& furniture);
    void send_message(const ServerDayEnd& day);
    void send_message(const ServerDay& day);
    void send_message(const ServerSleepingPlayers& sleepers);
    void send_message(const ServerClock& clock);
    void send_message(const ServerCrop& crop);
    void send_message(const ServerGroundEffect& ground);
    void send_message(const ServerChat& chat);
    void send_message(const ServerGiftRequest& gift);
    void send_message(const ServerGiftAnswered& answered);
    void send_message(const ServerGold& gold);
    void send_message(const ServerNpcPlacement& placement);
    void send_message(const ServerMapChange& change);
    void send_message(const ServerMarketOffer& offer);

public:
    explicit ProtocolSender(SocketIO& socket);
    
    void send_client_message(const protocol::ClientMessage& message);
    void send_server_message(const ServerMessage& message);
    void send_welcome(const ServerWelcome& welcome);
    void send_match_request(const MatchRequest& request);
    void send_lobby_status(LobbyStatus status);
    void send_world_list_request();
    void send_world_list(const std::vector<std::string>& worlds);
    void send_message(const protocol::GiftResponseMessage& message);
    void send_message(const ServerSetCompleted& completed);
    void send_message(const ServerVictory& victory);
};

#endif
