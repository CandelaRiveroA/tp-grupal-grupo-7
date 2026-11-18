#ifndef INVENTORY_H
#define INVENTORY_H

#include <cstdint>
#include <memory>
#include <vector>

#include "item.h"
#include "slot.h"

class Inventory {
private:
    uint8_t max_slots;
    std::vector<Slot> slots;

public:
    explicit Inventory(uint8_t max);
    ~Inventory() = default;

    Inventory(const Inventory&) = delete;
    Inventory& operator=(const Inventory&) = delete;
    Inventory(Inventory&&) = default;
    Inventory& operator=(Inventory&&) = default;

    bool add_item(std::unique_ptr<Item> new_item);
    bool is_slot_empty(uint8_t index) const;
    const Item* peek_item(uint8_t index) const;
    uint8_t get_amount(uint8_t index) const;
    uint8_t get_max_slots() const;
    void consume_item(uint8_t slot_idx);
    std::unique_ptr<Item> remove_item(uint8_t index);
    std::unique_ptr<Item> pop_one_item(uint8_t slot_idx);
};

#endif  // INVENTORY_H
