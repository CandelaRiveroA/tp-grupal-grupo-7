#include "tool.h"

Tool::Tool(ItemID id) : Item(id, false) {}

std::unique_ptr<Item> Tool::clone() const {
    return std::make_unique<Tool>(this->id);
}
