#ifndef ITEM_H
#define ITEM_H

#include "Item_Types.h"

class Item {
protected:
    ItemID id;
    bool is_tradable;

public:
    Item(ItemID id, bool tradable);
    virtual ~Item() = default; 
    Item(const Item&) = delete;
    Item& operator=(const Item&) = delete;
    Item(Item&&) = default;
    Item& operator=(Item&&) = default;

    ItemID get_id() const;
    bool can_be_traded() const;
    
    virtual bool is_stackable_with(const Item& other) const;
};

#endif // ITEM_H