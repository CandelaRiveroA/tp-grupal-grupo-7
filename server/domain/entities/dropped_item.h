#ifndef DROPPED_ITEM_H
#define DROPPED_ITEM_H

#include <memory>

#include "../inventory/item.h"

#include "i_entity.h"

class DroppedItem: public IEntity {
private:
    std::unique_ptr<Item> payload;

public:
    DroppedItem(std::unique_ptr<Item> item, uint16_t x, uint16_t y);
    ~DroppedItem() override = default;

    bool is_walkable() const override;
    void interact(Player& p, ToolType tool, World& w) override;
    std::optional<CellSnapshot> snapshot() const override;
    bool pickup_on_take() const override;
};

#endif  // DROPPED_ITEM_H
