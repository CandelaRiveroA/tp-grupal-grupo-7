#include "product.h"
#include <typeinfo> // para que es el type info

Product::Product(ItemID id, uint16_t base_price, Quality quality) 
    : Item(id, true), base_price(base_price), quality(quality) {}

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
