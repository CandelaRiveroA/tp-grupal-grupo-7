#include "chicken.h"
#include "../map/world.h"
#include "../inventory/product.h"
#include "../entities/dropped_item.h" 
#include <memory>

Chicken::Chicken(uint16_t x, uint16_t y) : Animal(x, y) {}

void Chicken::update(World& w) {
    this->days_passed++;
    
    // Las gallinas ponen 1 huevo todos los días sin esperar interacción
    if (this->days_passed >= 1) {
        this->produce(w); 
    }
}

void Chicken::produce(World& w) {
    auto egg = std::make_unique<Product>(ItemID::EGG, 3, Quality::BASE);
    
    auto dropped_egg = std::make_unique<DroppedItem>(this->get_x(), this->get_y(), std::move(egg));
    
    w.set_entity_at(this->get_x(), this->get_y(), std::move(dropped_egg)); 
    
    this->days_passed = 0;
}

void Chicken::interact(Player& p, ToolType tool, World& w) {}