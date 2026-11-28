#include "commands/cmd/item_commands.h"

#include <stdexcept>
#include <utility>

#include "domain/inventory/decoration_item.h"
#include "domain/inventory/product.h"
#include "domain/inventory/tool.h"
#include "domain/map/world.h"

namespace {

using Factory = std::function<std::unique_ptr<Item>(Quality)>;

Factory product(ItemID id, uint16_t price) {
    return [id, price](Quality quality) { return std::make_unique<Product>(id, price, quality); };
}

Factory tool(ItemID id) {
    return [id](Quality) { return std::make_unique<Tool>(id); };
}

Factory furniture(ItemID id, uint16_t model = 1) {
    return [id, model](Quality) { return std::make_unique<DecorationItem>(id, model); };
}

const std::map<std::string, Quality> qualities = {
    {"1", Quality::BASE}, {"base", Quality::BASE},
    {"2", Quality::GOOD}, {"buena", Quality::GOOD},
    {"3", Quality::EXCELLENT}, {"excelente", Quality::EXCELLENT},
    {"4", Quality::INCREDIBLE}, {"increible", Quality::INCREDIBLE},
    {"5", Quality::PERFECT}, {"perfecta", Quality::PERFECT}
};

}  // namespace

namespace commands {

ItemCommands::ItemCommands(): items_{
    {"pico", tool(ItemID::PICKAXE)},
    {"pala", tool(ItemID::SHOVEL)},
    {"regadera", tool(ItemID::WATERING_CAN)},
    {"espada", tool(ItemID::SWORD)},
    {"cofre", furniture(ItemID::CHEST)},
    {"decoracion_1", furniture(ItemID::DECORATION, 1)},
    {"decoracion_2", furniture(ItemID::DECORATION, 2)},
    {"frutilla", product(ItemID::STRAWBERRY, Product::base_price_for(ItemID::STRAWBERRY))},
    {"cebolla", product(ItemID::GREEN_ONION, Product::base_price_for(ItemID::GREEN_ONION))},
    {"papa", product(ItemID::POTATO, Product::base_price_for(ItemID::POTATO))},
    {"calabaza", product(ItemID::PUMPKIN, Product::base_price_for(ItemID::PUMPKIN))},
    {"semilla_frutilla", product(ItemID::STRAWBERRY_SEED, 20)},
    {"semilla_cebolla", product(ItemID::ONION_SEED, 20)},
    {"semilla_papa", product(ItemID::POTATO_SEED, 20)},
    {"semilla_calabaza", product(ItemID::PUMPKIN_SEED, 20)},
    {"leche", product(ItemID::MILK, Product::base_price_for(ItemID::MILK))},
    {"huevo", product(ItemID::EGG, Product::base_price_for(ItemID::EGG))},
    {"carbon", product(ItemID::COAL_ORB, 10)},
    {"lapislazuli", product(ItemID::LAPIS_ORB, 100)},
    {"plata", product(ItemID::SILVER_ORB, 250)},
    {"oro", product(ItemID::GOLD_ORB, 750)}
} {}

bool ItemCommands::parse_quantity(const std::string& text, uint16_t& quantity) {
    try {
        std::size_t end = 0;
        const int value = std::stoi(text, &end);
        if (end != text.size() || value < 1 || value > 65535) return false;
        quantity = static_cast<uint16_t>(value);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string ItemCommands::dar(World& world, uint8_t sender, const std::vector<std::string>& args) {
    if (args.size() < 2 || args.size() > 3) return "uso: /dar [objeto] [calidad] {cantidad}";
    const auto item = items_.find(args[0]);
    if (item == items_.end()) return "objeto desconocido; consulta /dar_info";
    const auto quality = qualities.find(args[1]);
    if (quality == qualities.end()) return "calidad invalida: 1/base, 2/buena, 3/excelente, 4/increible, 5/perfecta";
    uint16_t quantity = 1;
    if (args.size() == 3 && !parse_quantity(args[2], quantity)) return "cantidad invalida: debe ser de 1 a 65535";

    auto sample = item->second(quality->second);
    if (sample->get_quality() != quality->second) return "este objeto solo admite calidad 1/base";
    Player& player = world.get_player(sender);
    uint32_t added = 0;
    for (; added < quantity; ++added) {
        if (!world.try_pick_up_item(player, added == 0 ? std::move(sample) : item->second(quality->second))) break;
    }
    std::string result = "objetos agregados: " + std::to_string(added) + "/" + std::to_string(quantity);
    if (added < quantity) result += " (inventario lleno)";
    return result;
}

std::string ItemCommands::dar_info(World&, uint8_t, const std::vector<std::string>& args) {
    if (!args.empty()) return "uso: /dar_info";
    std::string result = "Objetos disponibles: ";
    for (const auto& item: items_) {
        if (result.back() != ' ') result += ", ";
        result += item.first;
    }
    result += ". Calidad: 1/base, 2/buena, 3/excelente, 4/increible, 5/perfecta.";
    return result;
}

}  // namespace commands
