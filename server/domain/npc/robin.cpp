#include "robin.h"

#include <algorithm>
#include <memory>

#include "../inventory/decoration_item.h"

Robin::Robin(uint16_t x, uint16_t y):
        Merchant(x, y, "Robin"), full_pool(), current_offset(0) {
    // Ajustá los ItemID y model_id según tu enum ItemID actual
    full_pool = {
        {"cofre", {ItemID::CHEST, 100, []() {
            return std::make_unique<DecorationItem>(ItemID::CHEST, 1);
        }}},
        {"decoracion_1", {ItemID::DECORATION, 50, []() {
            return std::make_unique<DecorationItem>(ItemID::DECORATION, 1);
        }}},
        {"decoracion_2", {ItemID::DECORATION, 60, []() {
            return std::make_unique<DecorationItem>(ItemID::DECORATION, 2);
        }}},
        // Agregá acá el resto de muebles/decoraciones
    };

    on_new_day();
}

// Por ahora no se usará debido a justo tener la cantidad 
// minima de decorativos para el catalogo
void Robin::on_new_day() {
    sell_catalog.clear();
    if (full_pool.empty()) {
        return;
    }

    const std::size_t daily_limit = std::min<std::size_t>(7, full_pool.size());
    for (std::size_t i = 0; i < daily_limit; ++i) {
        const auto& [item_name, entry] = full_pool[(current_offset + i) % full_pool.size()];
        sell_catalog[item_name] = entry;
    }

    current_offset = (current_offset + daily_limit) % full_pool.size();
}