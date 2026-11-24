#ifndef SERVER_COMMANDS_CMD_MERCHANT_COMMANDS_H
#define SERVER_COMMANDS_CMD_MERCHANT_COMMANDS_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "common/common_server_status.h"
#include "domain/map/world.h"

namespace commands {

class MerchantCommands {
private:
    std::map<std::string, std::string> offer_names_;

    bool parse_int(const std::string& text, int& value) const;

public:
    MerchantCommands();

    std::string consultar_catalogo(World& world, uint8_t sender,
                                   const std::vector<std::string>& args);
    std::string comprar(World& world, uint8_t sender,
                        const std::vector<std::string>& args);
    std::string vender(World& world, uint8_t sender,
                       const std::vector<std::string>& args);
    std::optional<ServerMarketOffer> offer_for(World& world, uint8_t sender) const;
};

}  // namespace commands

#endif  // SERVER_COMMANDS_CMD_MERCHANT_COMMANDS_H