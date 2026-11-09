#include "potato.h"
#include "../map/world.h"
#include "../inventory/product.h"
#include "../entities/dropped_item.h"
#include <memory>

Potato::Potato(uint16_t x, uint16_t y) 
    : Crop(x, y, 3, 2, ItemID::POTATO, 18) {} // 3 días, 2 riegos, 18 oro[cite: 1]

void Potato::harvest(World& w) {
    auto product = std::make_unique<Product>(this->product_id, this->base_price, this->current_quality);
    auto drop = std::make_unique<DroppedItem>(this->get_x(), this->get_y(), std::move(product));
    
    w.set_entity_at(this->get_x(), this->get_y(), std::move(drop));

    // Se retira de la grilla al ser recolectada
    w.remove_entity(this);
}
