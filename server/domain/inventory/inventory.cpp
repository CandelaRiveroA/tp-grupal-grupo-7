#include "inventory.h"
#include <utility>

Inventory::Inventory(uint8_t max) : max_slots(max) {
    this->slots.resize(this->max_slots);
}

bool Inventory::add_item(std::unique_ptr<Item> new_item) {
    if (!new_item) return false;

    // 1. Intentar APILAR
    for (auto& slot : this->slots) {
        if (!slot.is_empty()) {
            // Le preguntamos al ítem guardado si se puede apilar con el nuevo
            if (slot.item->is_stackable_with(*new_item)) {
                if (slot.amount < 50) {
                    slot.amount++;
                    return true; 
                }
            }
        }
    }

    // 2. Buscar slot VACÍO
    for (auto& slot : this->slots) {
        if (slot.is_empty()) {
            slot.item = std::move(new_item);
            slot.amount = 1;
            return true;
        }
    }

    return false;
}