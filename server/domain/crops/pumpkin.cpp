#include "pumpkin.h"
#include "../map/world.h"
#include "../inventory/product.h"
#include "../entities/dropped_item.h"
#include <memory>

Pumpkin::Pumpkin(uint16_t x, uint16_t y) 
    : Crop(x, y, 7, 1, ItemID::PUMPKIN, 50) {} // 7 días, 1 riego, 50 oro[cite: 1]

void Pumpkin::harvest(World& w) {
    auto product = std::make_unique<Product>(this->product_id, this->base_price, this->current_quality);
    auto drop = std::make_unique<DroppedItem>(std::move(product), this->get_x(), this->get_y());
    
    w.set_entity_at(this->get_x(), this->get_y(), std::move(drop));

    // Se retira de la grilla al ser recolectada
    w.remove_entity(this);
}