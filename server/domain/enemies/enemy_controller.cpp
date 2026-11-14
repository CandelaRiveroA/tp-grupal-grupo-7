#include "enemy_controller.h"

#include <array>
#include <functional>
#include <memory>
#include <random>

#include "../map/world.h"

#include "skeleton.h"
#include "skull.h"

// 1% de las celdas libres son puntos de aparición (5 en 30x20).
// shortcut: densidad placeholder, ajustar con partida real; un punto de manada o
// pareja puede generar varios enemigos.
#define kEnemyDensityPercent 1

// Salas de la caverna: la dificultad sube con la sala.
#define kTotalRooms 10
// Pesos por sala: el esqueleto baja de 100 a 10, el orco queda fijo y el
// vampiro sube de 10 a 100.
#define kSkeletonSlope 10
#define kOrcWeight 30
#define kVampireSlope 10

EnemyController::EnemyController(uint8_t sala, uint32_t seed):
        EntityController(kEnemyDensityPercent, seed), sala(sala), skull_placed(false) {}

void EnemyController::clear_generation(World& world) {
    // El mundo es el dueño de la lista de enemigos: él los identifica y los
    // retira de las celdas y de la lista de ticks (sin RTTI).
    this->skull_placed = false;
    world.clear_enemies();
}

void EnemyController::spawn_at(World& world, uint16_t x, uint16_t y) {
    // Todos los spawners entran por add_enemy: es lo que registra al enemigo
    // en la lista de ticks (tick_enemies) y lo que clear_enemies() limpia al
    // regenerar. set_entity_at solo pondría la entidad en la celda.
    if (!this->skull_placed) {
        this->skull_placed = true;
        world.add_enemy(std::make_unique<Skull>(x, y));
        return;
    }

    const std::array<int, 3> weights = {kSkeletonSlope * (kTotalRooms + 1 - this->sala),
                                        kOrcWeight, kVampireSlope * this->sala};

    using EnemySpawner = std::function<void(World&, uint16_t, uint16_t)>;
    static const std::array<EnemySpawner, 3> spawners = {
            [](World& w, uint16_t x, uint16_t y) {
                w.add_enemy(std::make_unique<Skeleton>(x, y));
            },
            [](World& w, uint16_t x, uint16_t y) { w.spawn_orc_pack(x, y); },
            [](World& w, uint16_t x, uint16_t y) { w.spawn_vampire_pair(x, y); }};

    std::discrete_distribution<size_t> pick(weights.begin(), weights.end());
    spawners[pick(this->rng)](world, x, y);
}
