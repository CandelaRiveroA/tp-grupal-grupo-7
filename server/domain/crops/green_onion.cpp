#include "green_onion.h"
#include "../map/world.h"
#include "../inventory/product.h"
#include "../entities/dropped_item.h"
#include <memory>

GreenOnion::GreenOnion(uint16_t x, uint16_t y) 
    : Crop(x, y, 2, 3, ItemID::GREEN_ONION, 12) {} // 2 días, 3 riegos, 12 oro

void GreenOnion::harvest(Player& p, World& w) {
    auto product = std::make_unique<Product>(this->product_id, this->base_price, this->current_quality);
    
    // 2. Intentamos dárselo al jugador directamente
    bool success = p.pick_up_item(std::move(product));
    
    // 3. Si el inventario tenía lugar, la planta se arranca de la tierra
    if (success) {
        w.record_pickup(p.get_id(), this->product_id);
        w.remove_entity(this);
    }
    // Si no se levanta es por inventario lleno, mandar alerta.
}