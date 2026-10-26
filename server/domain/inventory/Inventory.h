#ifndef INVENTORY_H
#define INVENTORY_H

#include <vector>
#include <memory>
#include <cstdint>
#include "Slot.h"
#include "Item.h"

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
};

#endif // INVENTORY_H