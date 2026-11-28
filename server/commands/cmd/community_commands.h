#ifndef SERVER_COMMANDS_CMD_COMMUNITY_COMMANDS_H
#define SERVER_COMMANDS_CMD_COMMUNITY_COMMANDS_H

#include <cstdint>
#include <string>
#include <vector>

#include "domain/map/world.h"

namespace commands {

class CommunityCommands {
public:
    std::string completar_set(World& world, uint8_t sender,
                              const std::vector<std::string>& args);
    std::string completar_todo(World& world, uint8_t sender,
                               const std::vector<std::string>& args);
};

}  // namespace commands

#endif  // SERVER_COMMANDS_CMD_COMMUNITY_COMMANDS_H
