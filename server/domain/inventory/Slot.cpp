#include "Slot.h"

Slot::Slot() : item(nullptr), amount(0) {}

bool Slot::is_empty() const {
    return this->item == nullptr;
}