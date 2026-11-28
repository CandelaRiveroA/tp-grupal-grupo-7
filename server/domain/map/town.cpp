#include "town.h"

#include <algorithm>
#include <map>
#include <string>
#include <utility>

#include "../inventory/product.h"
#include "../npc/marnie.h"
#include "../npc/robin.h"

Town::Town(uint16_t width, uint16_t height): Scene(width, height) {}

void Town::on_new_day(World& /*world*/) {
    for (Merchant* merchant: merchants) {
        merchant->on_new_day();
    }
}

static CatalogEntry make_seed_entry(ItemID id, uint16_t price) {
    return {id, price, [id, price]() {
        return std::make_unique<Product>(id, price, Quality::BASE);
    }};
}

static std::unique_ptr<Merchant> make_pierre(uint16_t x, uint16_t y) {
    std::map<std::string, CatalogEntry> sell = {
        {"semilla_frutilla", make_seed_entry(ItemID::STRAWBERRY_SEED, 20)},
        {"semilla_cebolla",  make_seed_entry(ItemID::ONION_SEED, 20)},
        {"semilla_papa",     make_seed_entry(ItemID::POTATO_SEED, 20)},
        {"semilla_calabaza", make_seed_entry(ItemID::PUMPKIN_SEED, 20)},
    };
    std::map<std::string, ItemID> buy = {
        {"frutilla", ItemID::STRAWBERRY},
        {"cebolla",  ItemID::GREEN_ONION},
        {"papa",     ItemID::POTATO},
        {"calabaza", ItemID::PUMPKIN},
        {"leche",    ItemID::MILK},
        {"huevo",    ItemID::EGG},
    };
    std::map<ItemID, uint16_t> buy_prices;
    for (const ItemID id: {ItemID::STRAWBERRY, ItemID::GREEN_ONION, ItemID::POTATO,
                           ItemID::PUMPKIN, ItemID::MILK, ItemID::EGG}) {
        buy_prices[id] = Product::base_price_for(id);
    }
    return std::make_unique<Merchant>(x, y, "Pierre", "pierre", std::move(sell), std::move(buy),
                                      std::move(buy_prices));
}

static std::unique_ptr<Merchant> make_clint(uint16_t x, uint16_t y) {
    std::map<std::string, ItemID> buy = {
        {"carbon",      ItemID::COAL_ORB},
        {"lapislazuli", ItemID::LAPIS_ORB},
        {"plata",       ItemID::SILVER_ORB},
        {"oro",         ItemID::GOLD_ORB},
    };
    return std::make_unique<Merchant>(x, y, "Clint", "clint", std::map<std::string, CatalogEntry>{}, std::move(buy));
}

void Town::place_default_merchants() {
    //TODO: conectar con archivo toml
    place_merchant(std::make_unique<Marnie>(4, 3));
    place_merchant(std::make_unique<Robin>(7, 3));
    place_merchant(make_pierre(10, 3));
    place_merchant(make_clint(13, 3));
}

std::unique_ptr<Merchant> Town::make_merchant(worldmodel::MarkerKind kind, uint16_t x, uint16_t y) {
    if (kind == worldmodel::MarkerKind::MerchantMarnie) {
        return std::make_unique<Marnie>(x, y);
    }
    if (kind == worldmodel::MarkerKind::MerchantRobin) {
        return std::make_unique<Robin>(x, y);
    }
    if (kind == worldmodel::MarkerKind::MerchantPierre) {
        return make_pierre(x, y);
    }
    if (kind == worldmodel::MarkerKind::MerchantClint) {
        return make_clint(x, y);
    }
    return nullptr;
}