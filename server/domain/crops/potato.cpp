#include "potato.h"
#include "../map/world.h"
#include "../inventory/product.h"
#include "../entities/dropped_item.h"
#include <memory>

Potato::Potato(uint16_t x, uint16_t y) 
    : Crop(x, y, 3, 2, ItemID::POTATO, 18) {} // 3 días, 2 riegos, 18 oro[cite: 1]

void Potato::harvest(Player& p, World& w) {
    auto product = std::make_unique<Product>(this->product_id, this->base_price, this->current_quality);
    
    // 2. Intentamos dárselo al jugador directamente
    bool success = p.pick_up_item(std::move(product));
    
    // 3. Si el inventario tenía lugar, la planta se arranca de la tierra
    if (success) {
        w.remove_entity(this);
    }
}
