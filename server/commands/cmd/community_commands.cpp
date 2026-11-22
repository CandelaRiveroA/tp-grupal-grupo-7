#include "commands/cmd/community_commands.h"

#include <stdexcept>

namespace commands {

CommunityCommands::CommunityCommands(): item_names_{
        {"frutilla", ItemID::STRAWBERRY},
        {"cebolla", ItemID::GREEN_ONION},
        {"papa", ItemID::POTATO},
        {"calabaza", ItemID::PUMPKIN},
        {"carbon", ItemID::COAL_ORB},
        {"lapislazuli", ItemID::LAPIS_ORB},
        {"plata", ItemID::SILVER_ORB},
        {"oro", ItemID::GOLD_ORB},
        {"leche", ItemID::MILK},
        {"huevo", ItemID::EGG}} {}

bool CommunityCommands::parse_int(const std::string& text, int& value) const {
    try {
        std::size_t pos = 0;
        const int parsed = std::stoi(text, &pos);
        if (pos != text.size()) {
            return false;
        }
        value = parsed;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

ItemID CommunityCommands::item_from_name(const std::string& name) const {
    const auto it = item_names_.find(name);
    return it != item_names_.end() ? it->second : ItemID::NONE;
}

std::string CommunityCommands::donar(World& world, uint8_t sender,
                                     const std::vector<std::string>& args) {
    if (args.empty()) {
        return "uso: /donar [set] [item] [cantidad] (/donar dinero [cantidad])";
    }
    if (args[0] == "dinero") {
        if (args.size() != 2) return "uso: /donar dinero [cantidad]";
        int quantity = 0;
        if (!parse_int(args[1], quantity) || quantity <= 0 ||
            quantity > 65535) {
            return "cantidad invalida";
        }
        if (!world.donate_to_community_center(
                sender, "dinero", ItemID::GOLD_ORB,
                static_cast<uint16_t>(quantity))) {
            return "no tenes suficiente oro";
        }
        return "donacion registrada";
    }
    if (args.size() != 3) return "uso: /donar [set] [item] [cantidad]";
    int quantity = 0;
    if (!parse_int(args[2], quantity) || quantity <= 0 ||
        quantity > 65535) {
        return "cantidad invalida";
    }
    const ItemID item = item_from_name(args[1]);
    if (item == ItemID::NONE) return "item desconocido";
    if (!world.donate_to_community_center(
            sender, args[0], item, static_cast<uint16_t>(quantity))) {
        return "no tenes suficientes items para donar";
    }
    return "donacion registrada";
}

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
