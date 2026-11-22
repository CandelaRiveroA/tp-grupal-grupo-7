#include "marnie.h"

#include "../entities/player.h"
#include "../map/world.h"

Marnie::Marnie(uint16_t x, uint16_t y): Merchant(x, y, "Marnie") {
    // make_item = nullptr porque los animales no van al inventario sino a la granja
    sell_catalog["vacas"]    = {ItemID::COW, 80, nullptr};
    sell_catalog["gallinas"] = {ItemID::CHICKEN, 20, nullptr};

    buy_catalog["vacas"]     = ItemID::COW;
    buy_catalog["gallinas"]  = ItemID::CHICKEN;
}

std::string Marnie::handle_buy(Player& p, const std::string& item_name, uint16_t qty,
                               World& w) {
    const auto it = sell_catalog.find(item_name);
    if (it == sell_catalog.end()) {
        return "Marnie: Solo vendo vacas y gallinas.";
    }

    uint16_t total_cost = static_cast<uint16_t>(it->second.price * qty);
    if (w.is_market_discount_active()) {
        total_cost = static_cast<uint16_t>(total_cost / 2);
    }
    if (!p.try_remove_gold(total_cost)) {
        return "Marnie: No te alcanza el oro (necesitás " + std::to_string(total_cost) + "g).";
    }

    // TODO (al crear Farm): Tell, Don't Ask contra World -> Farm
    // if (!w.try_spawn_animals(it->second.id, qty)) {
    //     p.add_gold(total_cost);
    //     return "Marnie: ¡La granja está llena! No entran más " + item_name + ".";
    // }

    return "Marnie: ¡Listo! Envié " + std::to_string(qty) + " " + item_name + " a tu granja.";
}

std::string Marnie::handle_sell(Player& p, const std::string& item_name, uint16_t qty,
                                World& w) {
    const auto it = sell_catalog.find(item_name);
    if (it == sell_catalog.end()) {
        return "Marnie: Solo compro animales de granja.";
    }

    // TODO (al crear Farm):
    // if (!w.try_remove_animals(it->second.id, qty)) {
    //     return "Marnie: No tenés " + std::to_string(qty) + " " + item_name + " en tu granja.";
    // }

    const uint16_t sell_price = static_cast<uint16_t>(it->second.price / 2);
    uint16_t total_earned = static_cast<uint16_t>(sell_price * qty);
    if (w.is_double_sell_active()) {
        total_earned = static_cast<uint16_t>(total_earned * 2);
    }
    p.add_gold(total_earned);

    return "Marnie: ¡Gracias! Te pagué " + std::to_string(total_earned) + "g.";
}