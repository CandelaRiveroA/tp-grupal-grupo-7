#include "merchant.h"

#include <cstdlib>
#include <sstream>
#include <utility>
#include <vector>

#include "../entities/player.h"
#include "../inventory/product.h"
#include "../map/world.h"

Merchant::Merchant(uint16_t x, uint16_t y, std::string name):
        IEntity(x, y), name(std::move(name)), id() {}

Merchant::Merchant(uint16_t x, uint16_t y, std::string name, std::string id):
        IEntity(x, y), name(std::move(name)), id(std::move(id)) {}

Merchant::Merchant(uint16_t x, uint16_t y, std::string name,
                   std::map<std::string, CatalogEntry> sell_catalog,
                   std::map<std::string, ItemID> buy_catalog):
        IEntity(x, y),
        name(std::move(name)),
        sell_catalog(std::move(sell_catalog)),
        buy_catalog(std::move(buy_catalog)) {}

Merchant::Merchant(uint16_t x, uint16_t y, std::string name, std::string id,
                   std::map<std::string, CatalogEntry> sell_catalog,
                   std::map<std::string, ItemID> buy_catalog):
        IEntity(x, y),
        name(std::move(name)),
        id(std::move(id)),
        sell_catalog(std::move(sell_catalog)),
        buy_catalog(std::move(buy_catalog)) {}

bool Merchant::is_in_range(uint16_t x, uint16_t y) const {
    return std::abs(static_cast<int>(get_x()) - static_cast<int>(x)) <=
                   kMerchantInteractionRadius &&
           std::abs(static_cast<int>(get_y()) - static_cast<int>(y)) <=
                   kMerchantInteractionRadius;
}

bool Merchant::is_walkable() const { return false; }

void Merchant::update(World& /*w*/) {}

void Merchant::interact(Player& /*p*/, ToolType /*tool*/, World& /*w*/) {
}

const std::string& Merchant::get_id() const { return id; }
const std::string& Merchant::get_name() const { return name; }
const std::map<std::string, CatalogEntry>& Merchant::get_sell_catalog() const {
    return sell_catalog;
}
const std::map<std::string, ItemID>& Merchant::get_buy_catalog() const {
    return buy_catalog;
}

std::string Merchant::process_command(Player& p, const std::string& raw_command, World& w) {
    std::istringstream iss(raw_command);
    std::string action;
    iss >> action;

    if (action == "/consultar_catalogo") {
        return "";
    }

    std::string item_name;
    uint16_t qty = 0;
    if (!(iss >> item_name >> qty) || qty == 0) {
        return name + ": Comando inválido. Uso: /comprar <item> <cant> o /vender <item> <cant>.";
    }

    if (action == "/comprar") {
        return handle_buy(p, item_name, qty, w);
    }
    if (action == "/vender") {
        return handle_sell(p, item_name, qty, w);
    }

    return name + ": No reconozco ese comando.";
}

std::string Merchant::handle_buy(Player& p, const std::string& item_name, uint16_t qty,
                                 World& w) {
    const auto it = sell_catalog.find(item_name);
    if (it == sell_catalog.end() || !it->second.make_item) {
        return name + ": No vendo '" + item_name + "'.";
    }

    const CatalogEntry& entry = it->second;
    uint16_t total_cost = static_cast<uint16_t>(entry.price * qty);
    if (w.is_market_discount_active()) {
        total_cost = static_cast<uint16_t>(total_cost / 2);
    }

    if (!p.try_remove_gold(total_cost)) {
        return name + ": No tenés suficiente oro (necesitás " + std::to_string(total_cost) + "g).";
    }

    uint16_t bought = 0;
    for (uint16_t i = 0; i < qty; ++i) {
        if (!p.pick_up_item(entry.make_item())) {
            break;
        }
        bought++;
    }

    if (bought < qty) {
        uint16_t refund = static_cast<uint16_t>(entry.price * (qty - bought));
        if (w.is_market_discount_active()) {
            refund = static_cast<uint16_t>(refund / 2);
        }
        p.add_gold(refund);
        if (bought == 0) {
            return name + ": No tenés espacio en el inventario.";
        }
        return name + ": Solo entraron " + std::to_string(bought) + " de " + item_name +
               ". Te devolví " + std::to_string(refund) + "g.";
    }

    return name + ": ¡Compra exitosa! Compraste " + std::to_string(qty) + " de " + item_name + ".";
}

std::string Merchant::handle_sell(Player& p, const std::string& item_name, uint16_t qty,
                                  World& w) {
    const auto it = buy_catalog.find(item_name);
    if (it == buy_catalog.end()) {
        return name + ": No compro '" + item_name + "'.";
    }

    std::vector<std::unique_ptr<Item>> sold = p.extract_items(it->second, qty);
    if (sold.empty()) {
        return name + ": No tenés " + std::to_string(qty) + " unidades de " + item_name + ".";
    }

    uint16_t total_earned = 0;
    for (const auto& item: sold) {
        const Product* prod = dynamic_cast<const Product*>(item.get());
        if (prod != nullptr) {
            total_earned += prod->get_final_price();
        }
    }
    if (w.is_double_sell_active()) {
        total_earned = static_cast<uint16_t>(total_earned * 2);
    }

    p.add_gold(total_earned);
    return name + ": ¡Venta exitosa! Recibiste " + std::to_string(total_earned) + "g.";
}
