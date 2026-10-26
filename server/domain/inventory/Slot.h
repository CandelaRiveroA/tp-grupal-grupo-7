#ifndef SLOT_H
#define SLOT_H

#include <memory>
#include <cstdint>
#include "Item.h"

struct Slot {
    std::unique_ptr<Item> item;
    uint8_t amount;
    Slot();
    ~Slot() = default;
    Slot(const Slot&) = delete;
    Slot& operator=(const Slot&) = delete;

    Slot(Slot&&) = default;
    Slot& operator=(Slot&&) = default;

    bool is_empty() const;
};

#endif // SLOT_H