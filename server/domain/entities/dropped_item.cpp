#include "dropped_item.h"

#include <set>
#include <utility>

#include "../map/world.h"

#include "player.h"

DroppedItem::DroppedItem(std::unique_ptr<Item> item, uint16_t x, uint16_t y):
        IEntity(x, y), payload(std::move(item)) {}

bool DroppedItem::is_walkable() const { return true; }

std::optional<CellSnapshot> DroppedItem::snapshot() const {
    if (!payload) {
        return std::nullopt;
    }
    return CellSnapshot{CellSnapshot::Kind::Dropped, payload->get_id(), 0, 0};
}

bool DroppedItem::pickup_on_take() const {
    static const std::set<ItemID> kTakePickups = {ItemID::EGG};
    return this->payload != nullptr && kTakePickups.count(this->payload->get_id()) > 0;
}

void DroppedItem::interact(Player& p, ToolType tool, World& w) {
    if (tool == ToolType::HAND && this->payload) {
        // El payload se mueve al inventario: el item se lee antes.
        const ItemID picked = this->payload->get_id();

        if (p.pick_up_item(std::move(this->payload))) {
            w.remove_entity(this);
            w.record_pickup(p.get_id(), picked);
        }
    }
}

void DroppedItem::update(World& /*w*/) {} 