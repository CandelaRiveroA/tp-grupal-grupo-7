#include "green_onion.h"
#include "../map/world.h"
#include "../inventory/product.h"
#include "../entities/dropped_item.h"
#include <memory>

GreenOnion::GreenOnion(uint16_t x, uint16_t y) 
    : Crop(x, y, 2, 3, ItemID::GREEN_ONION, 12) {} // 2 días, 3 riegos, 12 oro

void GreenOnion::harvest(World& w) {
    auto product = std::make_unique<Product>(this->product_id, this->base_price, this->current_quality);
    auto drop = std::make_unique<DroppedItem>(this->get_x(), this->get_y(), std::move(product));
    
    w.set_entity_at(this->get_x(), this->get_y(), std::move(drop));

    w.remove_entity(this);
}