#include "commands/cmd/community_commands.h"

namespace commands {

std::string CommunityCommands::completar_set(
        World& world, uint8_t /*sender*/, const std::vector<std::string>& args) {
    if (args.size() != 1) {
        return "uso: /completar_set [cultivos|minerales|metales|animales|pelea|dinero]";
    }
    if (!world.complete_set(args[0])) {
        return "set desconocido o ya completado";
    }
    return "set completado";
}

std::string CommunityCommands::completar_todo(
        World& world, uint8_t /*sender*/, const std::vector<std::string>& args) {
    if (!args.empty()) return "uso: /completar_todo";
    if (!world.complete_all_sets()) {
        return "todos los sets ya estan completos";
    }
    return "centro comunitario completado";
}

}  // namespace commands
