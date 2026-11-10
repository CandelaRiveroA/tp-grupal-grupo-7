#include "dropped_item.h"

#include <utility>

#include "../map/world.h"

#include "player.h"

DroppedItem::DroppedItem(std::unique_ptr<Item> item, uint16_t x, uint16_t y):
        IEntity(x, y), payload(std::move(item)) {}

const Item* DroppedItem::get_payload() const { return this->payload.get(); }

bool DroppedItem::is_walkable() const { return true; }

std::optional<CellSnapshot> DroppedItem::snapshot() const {
    if (!payload) {
        return std::nullopt;
    }
    return CellSnapshot{CellSnapshot::Kind::Dropped, payload->get_id(), 0, 0};
}

void DroppedItem::interact(Player& p, ToolType tool, World& w) {
    // Dejamos la mano para los huevos
    // Pero tal bien se pueden levantar solo caminando.
    if (tool == ToolType::HAND && this->payload) {

        if (p.pick_up_item(std::move(this->payload))) {
            w.remove_entity(this);
        }
    }
}

void DroppedItem::update(World& /*w*/) {} 