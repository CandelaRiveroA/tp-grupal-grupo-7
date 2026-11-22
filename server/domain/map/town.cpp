#include "town.h"

#include <algorithm>
#include <map>
#include <string>
#include <utility>

#include "../inventory/product.h"
#include "../npc/marnie.h"
#include "../npc/robin.h"

Town::Town(uint16_t width, uint16_t height):
        width(width), height(height), grid(), merchants() {
    grid.reserve(height);
    for (uint16_t y = 0; y < height; ++y) {
        std::vector<Cell> row;
        row.reserve(width);
        for (uint16_t x = 0; x < width; ++x) {
            row.emplace_back();
        }
        grid.push_back(std::move(row));
    }
    place_default_merchants();
}

uint16_t Town::get_width() const { return width; }
uint16_t Town::get_height() const { return height; }

bool Town::in_bounds(uint16_t x, uint16_t y) const { return x < width && y < height; }

bool Town::is_walkable(uint16_t x, uint16_t y) const {
    if (!in_bounds(x, y)) {
        return false;
    }
    const Cell& cell = grid[y][x];
    if (cell.is_there_a_wall()) {
        return false;
    }
    for (const auto& ent: cell.entities) {
        if (!ent->is_walkable()) {
            return false;
        }
    }
    return true;
}

void Town::set_wall(uint16_t x, uint16_t y, bool wall) {
    if (in_bounds(x, y)) {
        grid[y][x].is_wall = wall;
    }
}

void Town::set_entity_at(uint16_t x, uint16_t y, std::unique_ptr<IEntity> entity) {
    if (!in_bounds(x, y) || !entity) {
        return;
    }
    entity->set_position(x, y);
    grid[y][x].entities.push_back(std::move(entity));
}

void Town::remove_entity(IEntity* target_entity) {
    if (!target_entity) {
        return;
    }
    const uint16_t x = target_entity->get_x();
    const uint16_t y = target_entity->get_y();
    if (!in_bounds(x, y)) {
        return;
    }

    auto& ents = grid[y][x].entities;
    ents.erase(std::remove_if(ents.begin(), ents.end(),
                              [target_entity](const std::unique_ptr<IEntity>& ent) {
                                  return ent.get() == target_entity;
                              }),
               ents.end());
}

std::vector<IEntity*> Town::get_entities_at(uint16_t x, uint16_t y) const {
    std::vector<IEntity*> result;
    if (!in_bounds(x, y)) {
        return result;
    }
    const Cell& cell = grid[y][x];
    result.reserve(cell.entities.size());
    for (const auto& ent: cell.entities) {
        result.push_back(ent.get());
    }
    return result;
}

Merchant* Town::get_merchant_at(uint16_t x, uint16_t y) {
    if (!in_bounds(x, y)) {
        return nullptr;
    }
    for (const auto& ent: grid[y][x].entities) {
        Merchant* merchant = dynamic_cast<Merchant*>(ent.get());
        if (merchant != nullptr) {
            return merchant;
        }
    }
    return nullptr;
}

void Town::on_new_day() {
    for (Merchant* merchant: merchants) {
        merchant->on_new_day();
    }
}

void Town::place_default_merchants() {
    // 1. Marnie (animales)
    auto marnie = std::make_unique<Marnie>(4, 3);
    merchants.push_back(marnie.get());
    set_entity_at(4, 3, std::move(marnie));

    // 2. Robin (muebles con rotación diaria)
    auto robin = std::make_unique<Robin>(7, 3);
    merchants.push_back(robin.get());
    set_entity_at(7, 3, std::move(robin));

    // 3. Pierre (semillas + cultivos/productos animales)
    std::map<std::string, CatalogEntry> pierre_sell = {
        {"semilla_frutilla", {ItemID::STRAWBERRY_SEED, 20, []() {
            return std::make_unique<Product>(ItemID::STRAWBERRY_SEED, 20, Quality::BASE);
        }}},
        {"semilla_cebolla", {ItemID::ONION_SEED, 20, []() {
            return std::make_unique<Product>(ItemID::ONION_SEED, 20, Quality::BASE);
        }}},
        {"semilla_papa", {ItemID::POTATO_SEED, 20, []() {
            return std::make_unique<Product>(ItemID::POTATO_SEED, 20, Quality::BASE);
        }}},
        {"semilla_calabaza", {ItemID::PUMPKIN_SEED, 20, []() {
            return std::make_unique<Product>(ItemID::PUMPKIN_SEED, 20, Quality::BASE);
        }}},
    };
    std::map<std::string, ItemID> pierre_buy = {
        {"frutilla", ItemID::STRAWBERRY},
        {"cebolla",  ItemID::GREEN_ONION},
        {"papa",     ItemID::POTATO},
        {"calabaza", ItemID::PUMPKIN},
        {"leche",    ItemID::MILK},
        {"huevo",    ItemID::EGG},
    };
    auto pierre = std::make_unique<Merchant>(10, 3, "Pierre",
                                             std::move(pierre_sell), std::move(pierre_buy));
    merchants.push_back(pierre.get());
    set_entity_at(10, 3, std::move(pierre));

    // 4. Clint (minerales)
    std::map<std::string, ItemID> clint_buy = {
        {"carbon",      ItemID::COAL_ORB},
        {"lapislazuli", ItemID::LAPIS_ORB},
        {"plata",       ItemID::SILVER_ORB},
        {"oro",         ItemID::GOLD_ORB},
    };
    auto clint = std::make_unique<Merchant>(13, 3, "Clint",
                                            std::map<std::string, CatalogEntry>{},
                                            std::move(clint_buy));
    merchants.push_back(clint.get());
    set_entity_at(13, 3, std::move(clint));
}
