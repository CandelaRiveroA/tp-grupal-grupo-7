#include "dropped_item.h"

#include <set>
#include <utility>

#include "../map/world.h"

#include "player.h"

DroppedItem::DroppedItem(std::unique_ptr<Item> item, uint16_t x, uint16_t y, uint8_t amount):
        IEntity(x, y), payload(std::move(item)), amount(amount) {}

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
        
        bool picked_at_least_one = false;
        const ItemID picked_id = this->payload->get_id(); 

        while (this->amount > 0) {
            std::unique_ptr<Item> copy = this->payload->clone();
            
            if (w.try_pick_up_item(p, std::move(copy))) {
                this->amount--;
                picked_at_least_one = true;
            } else {
                break;
            }
        }

        // Si la pila se vació a 0, destruimos el contenedor del suelo
        if (this->amount == 0) {
            w.remove_entity(this);
        }

        // Emitimos la señal de pickup UNA SOLA VEZ
        if (picked_at_least_one) {
            w.record_pickup(p.get_id(), picked_id);
        }
    }
}

void DroppedItem::update(World& /*w*/) {} 