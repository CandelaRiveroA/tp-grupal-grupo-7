#include "barn.h"

#include <algorithm>
#include <cstdlib>
#include <utility>

#include "../animals/animal.h"
#include "../entities/player.h"

Barn::Barn(uint16_t width, uint16_t height): Scene(width, height) {}

Barn::~Barn() = default;

std::size_t Barn::count_kind(const std::string& kind) const {
    return static_cast<std::size_t>(std::count_if(
            animals.begin(), animals.end(),
            [&kind](const Animal* animal) { return animal->kind_name() == kind; }));
}

void Barn::on_entity_removed(IEntity* entity) {
    Scene::on_entity_removed(entity);
    animals.erase(std::remove(animals.begin(), animals.end(), entity), animals.end());
}

void Barn::store_animal(std::unique_ptr<Animal> animal) {
    const uint16_t x = animal->get_x();
    const uint16_t y = animal->get_y();
    Animal* stored = animal.get();
    set_entity_at(x, y, std::move(animal));
    animals.push_back(stored);
}

bool Barn::add_animal(std::unique_ptr<Animal> animal) {
    if (!animal || !can_walk_to(animal->get_x(), animal->get_y())) {
        return false;
    }
    const auto type = std::find_if(
            animal_catalog.offers().begin(), animal_catalog.offers().end(),
            [&animal](const auto& entry) { return entry.second.kind == animal->kind_name(); });
    if (type == animal_catalog.offers().end() ||
        count_kind(type->second.kind) >= type->second.limit) {
        return false;
    }
    store_animal(std::move(animal));
    return true;
}

std::vector<std::pair<uint16_t, uint16_t>> Barn::spawn_cells(
        uint16_t quantity,
        const std::vector<std::pair<uint16_t, uint16_t>>& occupied) const {
    std::vector<std::pair<uint16_t, uint16_t>> cells;
    cells.reserve(quantity);
    for (uint16_t y = 0; y < height && cells.size() < quantity; ++y) {
        for (uint16_t x = 0; x < width && cells.size() < quantity; ++x) {
            const std::pair<uint16_t, uint16_t> cell{x, y};
            // Sin paredes, portales, huevos, animales ni jugadores.
            if (can_walk_to(x, y) && grid[y][x].entities.empty() &&
                std::find(occupied.begin(), occupied.end(), cell) == occupied.end()) {
                cells.push_back(cell);
            }
        }
    }
    return cells;
}

AnimalPurchaseResult Barn::buy_animals(
        Player& player, ItemID type, uint16_t quantity, uint16_t cost,
        const std::vector<std::pair<uint16_t, uint16_t>>& occupied) {
    if (quantity == 0) {
        return AnimalPurchaseResult::InvalidQuantity;
    }
    const AnimalOffer* found = animal_catalog.find(type);
    if (!found) {
        return AnimalPurchaseResult::UnknownAnimal;
    }
    const AnimalOffer& animal_type = *found;
    const std::size_t current = count_kind(animal_type.kind);
    if (current >= animal_type.limit || quantity > animal_type.limit - current) {
        return AnimalPurchaseResult::LimitReached;
    }
    const auto cells = spawn_cells(quantity, occupied);
    if (cells.size() != quantity) {
        return AnimalPurchaseResult::NoSpace;
    }
    if (player.get_gold() < cost) {
        return AnimalPurchaseResult::InsufficientGold;
    }

    // Preparar el lote y sus reservas antes de cobrar: la compra es completa.
    std::vector<std::unique_ptr<Animal>> purchased;
    purchased.reserve(quantity);
    animals.reserve(animals.size() + quantity);
    for (const auto& [x, y]: cells) {
        purchased.push_back(animal_type.create(x, y));
        grid[y][x].entities.reserve(grid[y][x].entities.size() + 1);
    }
    if (!player.try_remove_gold(cost)) {
        return AnimalPurchaseResult::InsufficientGold;
    }
    for (auto& animal: purchased) {
        store_animal(std::move(animal));
    }
    return AnimalPurchaseResult::Success;
}

void Barn::tick_animals(World& world) {
    for (Animal* animal: animals) {
        animal->wander(world);
    }
}

bool Barn::try_hand_action_animals(Player& player, World& world) {
    for (Animal* animal: animals) {
        const int dx = static_cast<int>(animal->get_x()) - static_cast<int>(player.get_x());
        const int dy = static_cast<int>(animal->get_y()) - static_cast<int>(player.get_y());
        if (std::abs(dx) <= 1 && std::abs(dy) <= 1 && animal->try_hand_action(player, world)) {
            return true;
        }
    }
    return false;
}

void Barn::on_new_day(World& world) {
    const std::vector<Animal*> animals_today = animals;
    for (Animal* animal: animals_today) {
        animal->update(world);
    }
}

bool Barn::try_remove_animals(ItemID type, uint16_t quantity) {
    const AnimalOffer* offer = animal_catalog.find(type);
    if (!offer || quantity == 0 || count_kind(offer->kind) < quantity) return false;

    // remove_entity actualiza también animals mediante on_entity_removed.
    // Iterar una copia evita invalidar el recorrido al borrar del registro.
    const std::vector<Animal*> available = animals;
    uint16_t remaining = quantity;
    for (Animal* animal: available) {
        if (remaining == 0) break;
        if (animal->kind_name() == offer->kind) {
            remove_entity(animal);
            --remaining;
        }
    }
    return true;
}
