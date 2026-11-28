#ifndef BARN_H
#define BARN_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../animals/animal_purchase.h"
#include "../animals/animal_catalog.h"
#include "../inventory/item_types.h"
#include "scene.h"

class Animal;

class Barn: public Scene {
private:
    std::vector<Animal*> animals;
    AnimalCatalog animal_catalog;

    std::size_t count_kind(const std::string& kind) const;
    void store_animal(std::unique_ptr<Animal> animal);
    std::vector<std::pair<uint16_t, uint16_t>> spawn_cells(
            uint16_t quantity,
            const std::vector<std::pair<uint16_t, uint16_t>>& occupied) const;

protected:
    void on_entity_removed(IEntity* entity) override;

public:
    Barn(uint16_t width, uint16_t height);
    ~Barn() override;

    // Inserción explícita para cargar entidades y preparar pruebas; no genera animales.
    bool add_animal(std::unique_ptr<Animal> animal);
    AnimalPurchaseResult buy_animals(
            Player& player, ItemID type, uint16_t quantity, uint16_t cost,
            const std::vector<std::pair<uint16_t, uint16_t>>& occupied);
    bool try_remove_animals(ItemID type, uint16_t quantity);
    std::size_t prepare_production(World& world);
    void tick_animals(World& world);
    bool try_hand_action_animals(Player& player, World& world);
    void on_new_day(World& world) override;
};

#endif  // BARN_H
