#include "merchant.h"

#include <sstream>
#include <utility>
#include <vector>

#include "../entities/player.h"
#include "../inventory/product.h"
#include "../map/world.h"

Merchant::Merchant(uint16_t x, uint16_t y, std::string name):
        IEntity(x, y), name(std::move(name)) {}

Merchant::Merchant(uint16_t x, uint16_t y, std::string name,
                   std::map<std::string, CatalogEntry> sell_catalog,
                   std::map<std::string, ItemID> buy_catalog):
        IEntity(x, y),
        name(std::move(name)),
        sell_catalog(std::move(sell_catalog)),
        buy_catalog(std::move(buy_catalog)) {}

bool Merchant::is_walkable() const { return false; }

void Merchant::update(World& /*w*/) {}

void Merchant::interact(Player& /*p*/, ToolType /*tool*/, World& /*w*/) {
    // Al cliquearlo, el NPC saluda e indica los comandos disponibles.
    // TODO: conectar con cola de mensajes de chat en World/Match.
}

std::string Merchant::process_command(Player& p, const std::string& raw_command, World& w) {
    std::istringstream iss(raw_command);
    std::string action;
    iss >> action;

    if (action == "/consultar_catalogo") {
        std::ostringstream out;
        out << "--- Catálogo de " << name << " ---\n";
        if (!sell_catalog.empty()) {
            out << "Vendo:\n";
            for (const auto& [item_name, entry]: sell_catalog) {
                out << " - " << item_name << ": " << entry.price << "g\n";
            }
        }
        if (!buy_catalog.empty()) {
            out << "Compro:\n";
            for (const auto& [item_name, id]: buy_catalog) {
                (void)id;
                out << " - " << item_name << "\n";
            }
        }
        return out.str();
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
                                 World& /*w*/) {
    const auto it = sell_catalog.find(item_name);
    if (it == sell_catalog.end() || !it->second.make_item) {
        return name + ": No vendo '" + item_name + "'.";
    }

    const CatalogEntry& entry = it->second;
    const uint16_t total_cost = static_cast<uint16_t>(entry.price * qty);

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
        const uint16_t refund = static_cast<uint16_t>(entry.price * (qty - bought));
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
                                  World& /*w*/) {
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

    p.add_gold(total_earned);
    return name + ": ¡Venta exitosa! Recibiste " + std::to_string(total_earned) + "g.";
}
