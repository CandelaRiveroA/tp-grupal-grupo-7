#ifndef SERVER_COMMANDS_CMD_ITEM_COMMANDS_H
#define SERVER_COMMANDS_CMD_ITEM_COMMANDS_H

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "domain/inventory/item.h"

class World;

namespace commands {

class ItemCommands {
private:
    using Factory = std::function<std::unique_ptr<Item>(Quality)>;
    std::map<std::string, Factory> items_;
    static bool parse_quantity(const std::string& text, uint16_t& quantity);

public:
    ItemCommands();
    std::string dar(World& world, uint8_t sender, const std::vector<std::string>& args);
    std::string dar_info(World& world, uint8_t sender, const std::vector<std::string>& args);
};

}  // namespace commands

#endif
