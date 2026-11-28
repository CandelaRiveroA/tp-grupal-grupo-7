#ifndef SERVER_COMMANDS_COMMANDS_H
#define SERVER_COMMANDS_COMMANDS_H

#include <cstdint>

#include "../domain/map/world.h"

#include "../../common/common_command.h"

// Cada comando del protocolo tiene su overload de apply. Si se agrega un
// alternativo a protocol::ClientMessage sin su overload, Match no compila.
namespace commands {

void apply(World& world, uint8_t player_id, const protocol::DonateCommunityMessage& message);

void apply(World& world, uint8_t player_id, const protocol::DiscardMessage& message);
void apply(World& world, uint8_t player_id, const protocol::OpenContainerMessage& message);
void apply(World& world, uint8_t player_id, const protocol::CloseContainerMessage& message);
void apply(World& world, uint8_t player_id, const protocol::ContainerTransferMessage& message);
void apply(World& world, uint8_t player_id, const protocol::PlaceFurnitureMessage& message);
void apply(World& world, uint8_t player_id, const protocol::GrabFurnitureMessage& message);
void apply(World& world, uint8_t player_id, const protocol::MoveMessage& message);
void apply(World& world, uint8_t player_id, const protocol::PickMessage& message);
void apply(World& world, uint8_t player_id, const protocol::TakeMessage& message);
void apply(World& world, uint8_t player_id, const protocol::SelectSlotMessage& message);
void apply(World& world, uint8_t player_id, const protocol::SleepMessage& message);
void apply(World& world, uint8_t player_id, const protocol::WakeMessage& message);
void apply(World& world, uint8_t player_id, const protocol::ChatMessage& message);
void apply(World& world, uint8_t player_id, const protocol::GiftResponseMessage& message);
void apply(World& world, uint8_t player_id, const protocol::ToggleWalkingMessage& message);
}  // namespace commands

#endif
