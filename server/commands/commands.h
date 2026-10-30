#ifndef SERVER_COMMANDS_COMMANDS_H
#define SERVER_COMMANDS_COMMANDS_H

#include <cstdint>

#include "domain/map/world.h"

#include "common_command.h"

// Cada comando del protocolo tiene su overload de apply. Si se agrega un
// alternativo a protocol::ClientMessage sin su overload, Match no compila.
namespace commands {

void apply(World& world, uint8_t player_id, const protocol::MoveMessage& message);
void apply(World& world, uint8_t player_id, const protocol::PickMessage& message);
void apply(World& world, uint8_t player_id, const protocol::TakeMessage& message);

}  // namespace commands

#endif
