#include "Item.h"

Item::Item(ItemID id, bool tradable) : id(id), is_tradable(tradable) {}

ItemID Item::get_id() const { return this->id; }

bool Item::can_be_traded() const { return this->is_tradable; }

// Para un ítem base, son apilables si tienen el mismo ID
bool Item::is_stackable_with(const Item& other) const {
    return this->id == other.get_id();
}