#include "inventory.h"

#include "product.h"
#include <stdexcept>
#include <utility>

Inventory::Inventory(uint8_t max): max_slots(max) { this->slots.resize(this->max_slots); }

bool Inventory::can_add_item(const Item& item) const {
    for (const auto& slot: slots) {
        if (slot.is_empty() || (!slot.is_full() && slot.item->is_stackable_with(item))) {
            return true;
        }
    }
    return false;
}

bool Inventory::add_item(std::unique_ptr<Item> new_item) {
    if (!new_item)
        return false;

    // 1. Intentar APILAR
    for (auto& slot: this->slots) {
        if (!slot.is_empty()) {
            if (slot.item->is_stackable_with(*new_item)) {
                if (!slot.is_full()) {
                    slot.amount++;
                    return true;
                }
            }
        }
    }

    // 2. Buscar slot VACÍO
    for (auto& slot: this->slots) {
        // cppcheck-suppress useStlAlgorithm
        if (slot.is_empty()) {
            slot.item = std::move(new_item);
            slot.amount = 1;
            return true;
        }
    }
    // TODO: De no poder apilar ni encontrar un slot vacio,
    // el item se pierde. Hay que cambiar la logica para que
    // el item se devuelva al sistema de drop o al jugador.
    return false;
}

bool Inventory::is_slot_empty(uint8_t index) const {
    if (index >= max_slots)
        throw std::out_of_range("Indice de slot invalido");
    return slots[index].is_empty();
}

const Item* Inventory::peek_item(uint8_t index) const {
    if (index >= max_slots || slots[index].is_empty())
        return nullptr;
    // .get() devuelve el puntero crudo del unique_ptr
    return slots[index].item.get();
}

uint8_t Inventory::get_max_slots() const { return this->max_slots; }

uint8_t Inventory::get_amount(uint8_t index) const {
    if (index >= max_slots || slots[index].is_empty())
        return 0;
    return slots[index].amount;
}

void Inventory::consume_item(uint8_t slot_idx) {
    if (slot_idx < this->max_slots && !this->slots[slot_idx].is_empty()) {
        this->slots[slot_idx].amount--;
        if (this->slots[slot_idx].amount == 0) {
            // Se gastó la última semilla, destruimos el puntero y vaciamos el slot
            this->slots[slot_idx].item.reset(); 
        }
    }
}

std::unique_ptr<Item> Inventory::remove_item(uint8_t index) {
    if (index >= this->max_slots || this->slots[index].is_empty()) {
        return nullptr;
    }

    std::unique_ptr<Item> extracted;

    if (this->slots[index].amount == 1) {
        extracted = std::move(this->slots[index].item);
        this->slots[index].amount = 0;
    } else {
        extracted = this->slots[index].item->clone();
        this->slots[index].amount--;
    }

    return extracted;
}

std::unique_ptr<Item> Inventory::pop_one_item(uint8_t slot_idx) {
    if (slot_idx >= this->max_slots || this->slots[slot_idx].is_empty()) {
        return nullptr;
    }

    std::unique_ptr<Item> item_to_return = this->slots[slot_idx].item->clone();

    this->slots[slot_idx].amount--;
    if (this->slots[slot_idx].amount == 0) {
        this->slots[slot_idx].item.reset();
    }

    return item_to_return;
}

std::vector<std::unique_ptr<Item>> Inventory::extract_items(ItemID id, uint16_t qty) {
    std::vector<std::unique_ptr<Item>> extracted;
    if (qty == 0) {
        return extracted;
    }

    uint16_t available = 0;
    for (const auto& slot: this->slots) {
        if (!slot.is_empty() && slot.item->get_id() == id && slot.item->can_be_traded()) {
            available += slot.amount;
        }
    }
    if (available < qty) {
        return extracted;
    }

    extracted.reserve(qty);
    uint16_t remaining = qty;
    for (uint8_t i = 0; i < this->max_slots && remaining > 0; ++i) {
        while (remaining > 0 && !this->slots[i].is_empty() &&
               this->slots[i].item->get_id() == id &&
               this->slots[i].item->can_be_traded()) {
            extracted.push_back(this->pop_one_item(i));
            --remaining;
        }
    }
    return extracted;
}

std::vector<std::unique_ptr<Item>> Inventory::extract_items_with_min_quality(
        ItemID id, Quality minimum_quality, uint16_t qty) {
    std::vector<std::unique_ptr<Item>> extracted;
    if (qty == 0) {
        return extracted;
    }
    uint16_t available = 0;
    for (const auto& slot: this->slots) {
        if (slot.is_empty() || slot.item->get_id() != id ||
            !slot.item->can_be_traded()) {
            continue;
        }
        if (slot.item->get_quality() >= minimum_quality) {
            available += slot.amount;
        }
    }
    if (available < qty) {
        return extracted;
    }

    extracted.reserve(qty);
    uint16_t remaining = qty;
    for (uint8_t i = 0; i < this->max_slots && remaining > 0; ++i) {
        while (remaining > 0 && !this->slots[i].is_empty() &&
               this->slots[i].item->get_id() == id) {
            if (!this->slots[i].item->can_be_traded()) {
                break;
            }
            if (this->slots[i].item->get_quality() < minimum_quality) {
                break;
            }
            extracted.push_back(this->pop_one_item(i));
            --remaining;
        }
    }
    return extracted;
}

bool Inventory::can_extract_items(ItemID id, uint16_t qty) const {
    if (qty == 0) {
        return false;
    }
    uint16_t available = 0;
    for (const auto& slot: this->slots) {
        if (!slot.is_empty() && slot.item->get_id() == id &&
            slot.item->can_be_traded()) {
            available += slot.amount;
        }
    }
    return available >= qty;
}

bool Inventory::can_extract_items_with_min_quality(
        ItemID id, Quality minimum_quality, uint16_t qty) const {
    if (qty == 0) {
        return false;
    }
    uint16_t available = 0;
    for (const auto& slot: this->slots) {
        if (slot.is_empty() || slot.item->get_id() != id ||
            !slot.item->can_be_traded()) {
            continue;
        }
        if (slot.item->get_quality() >= minimum_quality) {
            available += slot.amount;
        }
    }
    return available >= qty;
}

bool Inventory::transfer_one_to(Inventory& destination, uint8_t slot) {
    if (&destination == this || slot >= max_slots || slots[slot].is_empty() ||
        !destination.can_add_item(*slots[slot].item)) {
        return false;
    }
    return destination.add_item(remove_item(slot));
}
