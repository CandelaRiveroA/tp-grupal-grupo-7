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

    // Precio base y variantes de calidad de cada producto: fuente única para
    // quién lo produce y para el catálogo que lo muestra.
    static uint16_t base_price_for(ItemID id);
    static bool has_quality_variants(ItemID id);

    uint16_t get_final_price() const override;

    bool is_stackable_with(const Item& other) const override;
    Quality get_quality() const override;

    std::unique_ptr<Item> clone() const override;
};

#endif // PRODUCT_H