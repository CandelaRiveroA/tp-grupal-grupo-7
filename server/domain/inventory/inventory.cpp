#include "inventory.h"
#include <utility>
#include <stdexcept>

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
                if (!slot.is_full()) {
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

bool Inventory::is_slot_empty(uint8_t index) const {
    if (index >= max_slots) throw std::out_of_range("Indice de slot invalido");
    return slots[index].is_empty();
}

const Item* Inventory::peek_item(uint8_t index) const {
    if (index >= max_slots || slots[index].is_empty()) return nullptr;
    // .get() devuelve el puntero crudo del unique_ptr
    return slots[index].item.get(); 
}

uint8_t Inventory::get_amount(uint8_t index) const {
        if (index >= max_slots || slots[index].is_empty()) return 0;
        return slots[index].amount;
}