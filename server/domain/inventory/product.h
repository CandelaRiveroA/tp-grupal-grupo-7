#ifndef PRODUCT_H
#define PRODUCT_H

#include "item.h"
#include <cstdint>
#include <memory>

class Product : public Item {
private:
    uint16_t base_price;
    Quality quality;

public:
    Product(ItemID id, uint16_t base_price, Quality quality);
    ~Product() override = default;

    uint16_t get_final_price() const;

    bool is_stackable_with(const Item& other) const override;
    Quality get_quality() const;

    std::unique_ptr<Item> clone() const override;
};

#endif // PRODUCT_H