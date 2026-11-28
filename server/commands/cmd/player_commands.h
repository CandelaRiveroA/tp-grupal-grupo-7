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
    std::string invencibilidad(World& world, uint8_t sender, const std::vector<std::string>& args);
    std::string cuidar_animales(World& world, uint8_t sender,
                                const std::vector<std::string>& args);
    std::string cuidar_cultivos(World& world, uint8_t sender,
                                const std::vector<std::string>& args);
    std::string vida(World& world, uint8_t sender,
                     const std::vector<std::string>& args);
    std::string energia(World& world, uint8_t sender,
                        const std::vector<std::string>& args);
    std::string oro(World& world, uint8_t sender,
                    const std::vector<std::string>& args);
    std::string terminar_dia(World& world, uint8_t sender,
                             const std::vector<std::string>& args);
    std::string tiempo(World& world, uint8_t sender,
                       const std::vector<std::string>& args);
    std::string tp(World& world, uint8_t sender,
                   const std::vector<std::string>& args);
    std::string tp_info(World& world, uint8_t sender,
                        const std::vector<std::string>& args);
};

}  // namespace commands

#endif  // SERVER_COMMANDS_CMD_PLAYER_COMMANDS_H
