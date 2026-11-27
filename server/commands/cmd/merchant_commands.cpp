#include "commands/cmd/merchant_commands.h"

#include <stdexcept>

#include "domain/npc/merchant.h"

namespace commands {

MerchantCommands::MerchantCommands(): offer_names_{
        {"semilla_frutilla", "strawberry_seeds"},
        {"semilla_cebolla", "onion_seeds"},
        {"semilla_papa", "potato_seeds"},
        {"semilla_calabaza", "pumpkin_seeds"},
        {"frutilla", "strawberry"},
        {"cebolla", "onion"},
        {"papa", "potato"},
        {"calabaza", "pumpkin"},
        {"leche", "milk"},
        {"huevo", "egg"},
        {"carbon", "coal"},
        {"lapislazuli", "lapis"},
        {"plata", "silver"},
        {"oro", "gold"},
        {"vacas", "cow"},
        {"gallinas", "chicken"},
        {"cofre", "chest"},
        {"decoracion_1", "mesa_1"},
        {"decoracion_2", "mesa_2"}} {}

bool MerchantCommands::parse_int(const std::string& text, int& value) const {
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

std::string MerchantCommands::consultar_catalogo(
        World& world, uint8_t sender, const std::vector<std::string>& args) {
    if (!args.empty()) {
        return "uso: /consultar_catalogo";
    }
    return world.process_merchant_command(sender, "/consultar_catalogo");
}

std::string MerchantCommands::comprar(
        World& world, uint8_t sender, const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return "uso: /comprar <producto> <cantidad>";
    }
    int quantity = 0;
    if (!parse_int(args[1], quantity) ||
        quantity <= 0 || quantity > 65535) {
        return "cantidad invalida";
    }
    return world.process_merchant_command(
            sender, "/comprar " + args[0] + " " + args[1]);
}

std::string MerchantCommands::vender(
        World& world, uint8_t sender, const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return "uso: /vender <producto> <cantidad>";
    }
    int quantity = 0;
    if (!parse_int(args[1], quantity) ||
        quantity <= 0 || quantity > 65535) {
        return "cantidad invalida";
    }
    return world.process_merchant_command(
            sender, "/vender " + args[0] + " " + args[1]);
}

std::optional<ServerMarketOffer> MerchantCommands::offer_for(
        World& world, uint8_t sender) const {
    Merchant* merchant = world.get_merchant_for_command(sender);
    if (merchant == nullptr) {
        return std::nullopt;
    }

    std::vector<ServerMarketEntry> entries;
    for (const auto& [server_name, catalog_entry]: merchant->get_sell_catalog()) {
        const auto name = offer_names_.find(server_name);
        if (name == offer_names_.end()) {
            continue;
        }
        entries.push_back(ServerMarketEntry{name->second,
                                            server_name,
                                            ServerMarketSide::Sells,
                                            true,
                                            catalog_entry.price});
    }
    for (const auto& [server_name, item_id]: merchant->get_buy_catalog()) {
        (void)item_id;
        const auto name = offer_names_.find(server_name);
        if (name == offer_names_.end()) {
            continue;
        }
        entries.push_back(ServerMarketEntry{name->second,
                                            server_name,
                                            ServerMarketSide::Buys,
                                            false,
                                            0});
    }
    return ServerMarketOffer{merchant->get_id(), std::move(entries)};
}

}  // namespace commands
