#include "chicken.h"

#include <memory>
#include <utility>

#include "../entities/dropped_item.h"
#include "../inventory/product.h"
#include "../map/world.h"

Chicken::Chicken(uint16_t x, uint16_t y): Animal(x, y) {}

bool Chicken::is_walkable() const { return true; }

std::string Chicken::kind_name() const { return "chicken"; }

void Chicken::update(World& w) {
    this->days_passed++;

    // Las gallinas ponen 1 huevo todos los días sin esperar interacción
    if (this->days_passed >= 1) {
        this->produce(w);
    }
}

void Chicken::produce(World& w) {
    auto egg = std::make_unique<Product>(ItemID::EGG, Product::base_price_for(ItemID::EGG),
                                         Quality::BASE);

    // El huevo queda en la celda de la gallina, apilado con lo que ya haya.
    auto dropped_egg = std::make_unique<DroppedItem>(std::move(egg), this->get_x(), this->get_y());

    w.set_entity_at(this->get_x(), this->get_y(), std::move(dropped_egg));

    this->days_passed = 0;
}

void Chicken::interact(Player& /*p*/, ToolType /*tool*/, World& /*w*/) {}
