#include "marnie.h"

#include "../entities/player.h"
#include "../animals/animal_catalog.h"
#include "../map/world.h"

Marnie::Marnie(uint16_t x, uint16_t y):
        Merchant(x, y, "Marnie", "marnie"),
        purchase_errors{
                {AnimalPurchaseResult::InvalidQuantity, "La cantidad debe ser mayor que cero."},
                {AnimalPurchaseResult::UnknownAnimal, "Solo vendo vacas y gallinas."},
                {AnimalPurchaseResult::LimitReached,
                 "El establo admite como máximo 5 vacas y 10 gallinas."},
                {AnimalPurchaseResult::NoSpace, "No hay espacio libre para todos en el establo."},
                {AnimalPurchaseResult::BarnUnavailable, "No hay un establo disponible."},
        } {
    // Los animales se crean en el establo, no van al inventario.
    const AnimalCatalog animals;
    for (const auto& [id, offer]: animals.offers()) {
        sell_catalog[offer.command_name] = {id, offer.price, nullptr};
        buy_catalog[offer.command_name] = id;
    }
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
    const AnimalPurchaseResult result = w.buy_animals(p, it->second.id, qty, total_cost);
    if (result == AnimalPurchaseResult::InsufficientGold) {
        return "Marnie: No te alcanza el oro (necesitás " + std::to_string(total_cost) + "g).";
    }
    if (result != AnimalPurchaseResult::Success) {
        return "Marnie: " + purchase_errors.at(result);
    }
    return "Marnie: ¡Listo! Envié " + std::to_string(qty) + " " + item_name + " a tu establo.";
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
