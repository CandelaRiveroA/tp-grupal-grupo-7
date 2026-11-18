#include "decoration_item.h"

DecorationItem::DecorationItem(ItemID id, uint16_t model_id): Item(id, true), model_id(model_id) {}

uint16_t DecorationItem::get_model_id() const { return this->model_id; }

std::unique_ptr<Item> DecorationItem::clone() const {
    return std::make_unique<DecorationItem>(this->id, this->model_id);
}
