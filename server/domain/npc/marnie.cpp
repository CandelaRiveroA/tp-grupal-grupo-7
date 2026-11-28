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
    for (const auto& [id, offer]: animal_catalog.offers()) {
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
    const auto wanted = buy_catalog.find(item_name);
    if (wanted == buy_catalog.end()) {
        return "Marnie: Solo compro vacas y gallinas.";
    }
    const AnimalOffer* offer = animal_catalog.find(wanted->second);
    uint32_t total_earned = static_cast<uint32_t>(offer->sell_price) * qty;
    if (w.is_double_sell_active()) total_earned *= 2;
    if (total_earned > 65535u - p.get_gold()) {
        return "Marnie: No podés recibir tanto oro. No se realizó la venta.";
    }
    if (!w.try_remove_animals(wanted->second, qty)) {
        return "Marnie: No tenés " + std::to_string(qty) + " " + item_name + " en tu establo.";
    }
    p.add_gold(static_cast<uint16_t>(total_earned));
    return "Marnie: ¡Gracias! Te pagué " + std::to_string(total_earned) + "g.";
}

std::optional<uint16_t> Marnie::buy_price(ItemID item, uint8_t quality) const {
    // Los animales y sus productos solo existen en calidad.base: publicar
    // variantes superiores sería un precio que nadie podría cobrar.
    if (quality != 1) return std::nullopt;
    if (const AnimalOffer* offer = animal_catalog.find(item)) {
        return offer->sell_price;
    }
    return std::nullopt;
}
