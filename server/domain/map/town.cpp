#include "town.h"

#include <algorithm>
#include <map>
#include <string>
#include <utility>

#include "../inventory/product.h"
#include "../npc/marnie.h"
#include "../npc/robin.h"

Town::Town(uint16_t width, uint16_t height): Scene(width, height), merchants() {
    place_default_merchants();
}

void Town::on_entity_removed(IEntity* entity) {
    merchants.erase(std::remove(merchants.begin(), merchants.end(), entity), merchants.end());
}

Merchant* Town::get_merchant_at(uint16_t x, uint16_t y) {
    if (!in_bounds(x, y)) {
        return nullptr;
    }
    for (const auto& ent: this->grid[y][x].entities) {
        Merchant* merchant = dynamic_cast<Merchant*>(ent.get());
        if (merchant != nullptr) {
            return merchant;
        }
    }
    return nullptr;
}

Merchant* Town::get_nearest_merchant(uint16_t x, uint16_t y) const {
    Merchant* nearest = nullptr;
    int nearest_distance = 2;
    for (Merchant* merchant: merchants) {
        const int dx = std::abs(static_cast<int>(merchant->get_x()) - static_cast<int>(x));
        const int dy = std::abs(static_cast<int>(merchant->get_y()) - static_cast<int>(y));
        if (dx <= 1 && dy <= 1) {
            const int distance = dx + dy;
            if (distance < nearest_distance ||
                (distance == nearest_distance && nearest == nullptr)) {
                nearest = merchant;
                nearest_distance = distance;
            }
        }
    }
    return nearest;
}

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

void Town::place_merchant(std::unique_ptr<Merchant> merchant) {
    if (!merchant || !in_bounds(merchant->get_x(), merchant->get_y())) {
        return;
    }
    const uint16_t x = merchant->get_x();
    const uint16_t y = merchant->get_y();
    merchants.push_back(merchant.get());
    set_entity_at(x, y, std::move(merchant));
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
    return std::make_unique<Merchant>(x, y, "Pierre", "pierre", std::move(sell), std::move(buy));
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