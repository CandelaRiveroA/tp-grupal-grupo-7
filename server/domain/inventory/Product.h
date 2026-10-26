#ifndef PRODUCT_H
#define PRODUCT_H

#include "Item.h"
#include <cstdint>

class Product : public Item {
private:
    uint16_t base_price;
    Quality quality;

public:
    Product(ItemID id, uint16_t base_price, Quality quality);
    ~Product() override = default;

    uint16_t get_final_price() const;

    bool is_stackable_with(const Item& other) const override;
};

#endif // PRODUCT_H