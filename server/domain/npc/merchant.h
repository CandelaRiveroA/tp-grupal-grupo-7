#ifndef MERCHANT_H
#define MERCHANT_H

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>

#include "../entities/i_entity.h"
#include "../inventory/item.h"
#include "../inventory/item_types.h"

struct CatalogEntry {
    ItemID id;
    uint16_t price;
    // Fábrica del ítem a entregar al inventario (nullptr si es un animal de Marnie)
    std::function<std::unique_ptr<Item>()> make_item;
};

class Merchant: public IEntity {
protected:
    std::string name;
    std::map<std::string, CatalogEntry> sell_catalog;
    std::map<std::string, ItemID> buy_catalog;

    virtual std::string handle_buy(Player& p, const std::string& item_name, uint16_t qty, World& w);
    virtual std::string handle_sell(Player& p, const std::string& item_name, uint16_t qty, World& w);

public:
    Merchant(uint16_t x, uint16_t y, std::string name);
    Merchant(uint16_t x, uint16_t y, std::string name,
         std::map<std::string, CatalogEntry> sell_catalog,
         std::map<std::string, ItemID> buy_catalog);
    ~Merchant() override = default;

    Merchant(const Merchant&) = delete;
    Merchant& operator=(const Merchant&) = delete;
    Merchant(Merchant&&) = default;
    Merchant& operator=(Merchant&&) = default;

    bool is_walkable() const override;
    void interact(Player& p, ToolType tool, World& w) override;
    void update(World& w) override;

    std::string process_command(Player& p, const std::string& raw_command, World& w);

    virtual void on_new_day() {}
};

#endif  // MERCHANT_H
