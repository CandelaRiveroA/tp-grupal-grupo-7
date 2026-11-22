#ifndef SERVER_COMMANDS_CMD_PLAYER_COMMANDS_H
#define SERVER_COMMANDS_CMD_PLAYER_COMMANDS_H

#include <cstdint>
#include <string>
#include <vector>

#include "domain/map/world.h"

namespace commands {

class PlayerCommands {
private:
    bool parse_int(const std::string& text, int& value) const;

public:
    std::string vida(World& world, uint8_t sender,
                     const std::vector<std::string>& args);
    std::string energia(World& world, uint8_t sender,
                        const std::vector<std::string>& args);
    std::string oro(World& world, uint8_t sender,
                    const std::vector<std::string>& args);
};

}  // namespace commands

#endif  // SERVER_COMMANDS_CMD_PLAYER_COMMANDS_H
