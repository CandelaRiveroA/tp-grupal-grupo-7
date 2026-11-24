#ifndef FARM_H
#define FARM_H

#include <cstdint>
#include <memory>
#include <vector>

#include "scene.h"

class Crop;
class Player;
class World;

class Farm: public Scene {
private:
    std::vector<Crop*> crops;

protected:
    void on_entity_removed(IEntity* entity) override;

public:
    Farm(uint16_t width, uint16_t height);
    ~Farm() override = default;

    // --- Suelo y cultivos (Tell, Don't Ask sobre Cell) ---
    void till_soil(uint16_t x, uint16_t y);
    void water_soil(uint16_t x, uint16_t y);
    bool try_plant_seed(Player& player, uint16_t tx, uint16_t ty) override;

    // --- Fin / inicio de día ---
    void on_new_day(World& world) override;
};

#endif  // FARM_H
