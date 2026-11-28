#ifndef ANIMAL_CATALOG_H
#define ANIMAL_CATALOG_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>

#include "../inventory/item_types.h"

class Animal;

struct AnimalOffer {
    std::string command_name;
    std::string kind;
    std::size_t limit;
    uint16_t price;
    uint16_t sell_price;
    std::function<std::unique_ptr<Animal>(uint16_t, uint16_t)> create;
};

// Precios, topes y creación compartidos por compras, ventas y recompensas.
class AnimalCatalog {
private:
    std::map<ItemID, AnimalOffer> entries;

public:
    AnimalCatalog();
    const std::map<ItemID, AnimalOffer>& offers() const;
    const AnimalOffer* find(ItemID type) const;
};

#endif  // ANIMAL_CATALOG_H
