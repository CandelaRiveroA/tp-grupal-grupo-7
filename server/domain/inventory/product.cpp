#include "product.h"
#include <typeinfo>

Product::Product(ItemID id, uint16_t base_price, Quality quality) 
    : Item(id, true), base_price(base_price), quality(quality) {}

uint16_t Product::base_price_for(ItemID id) {
    switch (id) {
        case ItemID::STRAWBERRY: return 25;
        case ItemID::GREEN_ONION: return 12;
        case ItemID::POTATO: return 18;
        case ItemID::PUMPKIN: return 50;
        case ItemID::EGG: return 3;
        case ItemID::MILK: return 30;
        default: return 0;
    }
}

bool Product::has_quality_variants(ItemID id) {
    return id == ItemID::STRAWBERRY || id == ItemID::GREEN_ONION ||
           id == ItemID::POTATO || id == ItemID::PUMPKIN;
}

uint16_t Product::get_final_price() const {
    float multipliers[] = {1.0f, 1.25f, 2.0f, 4.0f, 8.0f};
    return static_cast<uint16_t>(this->base_price * multipliers[static_cast<int>(this->quality) - 1]);
}

bool Product::is_stackable_with(const Item& other) const {
    if (!Item::is_stackable_with(other)) {
        return false;
    }

    // Hacemos downcast seguro para ver si 'other' también es un Product
    // Usamos dynamic_cast porque necesitamos saber el tipo real en tiempo de ejecución.
    const Product* other_prod = dynamic_cast<const Product*>(&other);
    
    // Si 'other' es un producto, deben tener la misma calidad.
    // Si por algún motivo 'other' no fuera un Product (raro, porque tendrían distinto ID), retorna false.
    return other_prod != nullptr && this->quality == other_prod->quality;
}

Quality Product::get_quality() const { return this->quality; }

std::unique_ptr<Item> Product::clone() const {
    return std::make_unique<Product>(this->id, this->base_price, this->quality);
}

