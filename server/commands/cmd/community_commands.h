#ifndef SERVER_COMMANDS_CMD_COMMUNITY_COMMANDS_H
#define SERVER_COMMANDS_CMD_COMMUNITY_COMMANDS_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "domain/inventory/item_types.h"
#include "domain/map/world.h"

namespace commands {

class CommunityCommands {
private:
    std::map<std::string, ItemID> item_names_;

    bool parse_int(const std::string& text, int& value) const;
    ItemID item_from_name(const std::string& name) const;

public:
    CommunityCommands();

    std::string donar(World& world, uint8_t sender,
                      const std::vector<std::string>& args);
    std::string completar_set(World& world, uint8_t sender,
                              const std::vector<std::string>& args);
    std::string completar_todo(World& world, uint8_t sender,
                               const std::vector<std::string>& args);
};

}  // namespace commands

#endif  // SERVER_COMMANDS_CMD_COMMUNITY_COMMANDS_H
