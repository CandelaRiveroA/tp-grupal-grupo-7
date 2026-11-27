#include "mine_controller.h"

#include <array>
#include <functional>
#include <memory>
#include <vector>

#include "../entities/dropped_item.h"
#include "../map/world.h"

#include "coal_rock.h"
#include "gold_rock.h"
#include "lapis_rock.h"
#include "rock.h"
#include "silver_rock.h"

// % de celdas libres que ocupa una generación.
#define kMineDensityPercent 10

// Rates de aparición (suman 100, así que son % directos): roca = relleno
// de la sala, de ahí su frecuencia muy alta.
#define kWeightRock 45
#define kWeightCoal 27
#define kWeightLapis 15
#define kWeightSilver 8
#define kWeightGold 5

// Sala desde la que aparece el oro.
#define kGoldFromRoom 5

MineController::MineController(uint8_t sala, uint32_t seed):
        EntityController(kMineDensityPercent, seed), sala(sala) {}

void MineController::clear_generation(World& world) {
    std::vector<IEntity*> to_clear;
    for (uint16_t y = 0; y < world.get_height(); ++y) {
        for (uint16_t x = 0; x < world.get_width(); ++x) {
            for (IEntity* entity: world.get_entity_at(x, y)) {
                const auto snap = entity->snapshot();
                if (snap && (snap->kind == CellSnapshot::Kind::Resource ||
                             snap->kind == CellSnapshot::Kind::Dropped)) {
                    to_clear.push_back(entity);
                }
            }
        }
    }
    // Se recolecta primero: remove_entity borra del vector de la celda.
    for (IEntity* entity: to_clear) {
        world.remove_entity(entity);
    }
}

void MineController::spawn_at(World& world, uint16_t x, uint16_t y) {
    world.set_entity_at(x, y, make_rock(x, y));
}

std::unique_ptr<IEntity> MineController::make_rock(uint16_t x, uint16_t y) {
    const std::vector<int> weights = {kWeightRock, kWeightCoal, kWeightLapis, kWeightSilver,
                                      this->sala >= kGoldFromRoom ? kWeightGold : 0};

    using RockFactory = std::function<std::unique_ptr<IEntity>(uint16_t, uint16_t)>;
    static const std::array<RockFactory, 5> rock_factories = {
            [](uint16_t x, uint16_t y) { return std::make_unique<Rock>(x, y); },
            [](uint16_t x, uint16_t y) { return std::make_unique<CoalRock>(x, y); },
            [](uint16_t x, uint16_t y) { return std::make_unique<LapisRock>(x, y); },
            [](uint16_t x, uint16_t y) { return std::make_unique<SilverRock>(x, y); },
            [](uint16_t x, uint16_t y) { return std::make_unique<GoldRock>(x, y); }};

    std::discrete_distribution<size_t> pick(weights.begin(), weights.end());
    const size_t selected = pick(this->rng);

    if (selected < rock_factories.size()) {
        return rock_factories[selected](x, y);
    }

    return std::make_unique<Rock>(x, y);
}
