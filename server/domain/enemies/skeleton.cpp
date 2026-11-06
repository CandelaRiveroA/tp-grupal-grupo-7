#include "skeleton.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>

#include "../map/world.h"

// Placeholders de balance (dificultad baja). Ajustables.
#define kHealth 10
#define kDamage 1
// 15 ticks de CreatureTick (~4 s c/u) ≈ 60 s hasta el revive.
#define kReviveTicks 15
// Radio de persecución.
#define kAggroRange 5
// Bajo a propósito: revive, así que con más probabilidad sería granja de oro.
#define kGoldDropPercent 20

Skeleton::Skeleton(uint16_t x, uint16_t y): Enemy(x, y, kHealth, kDamage, kGoldDropPercent) {}

void Skeleton::on_death(World& /*w*/) {
    // Particularidad del esqueleto: no se retira del mundo, revive con vida
    // llena cuando update() se acaba el contador.
    this->ticks_to_revive = kReviveTicks;
}

void Skeleton::behave(World& w) {
    // Al jugador vivo más cercano dentro del radio de persecución.
    const Player* target = nullptr;
    int dx = 0;
    int dy = 0;
    int best_distance = kAggroRange + 1;
    for (const uint8_t id: w.get_player_ids()) {
        const Player& player = w.get_player(id);
        const int player_dx = static_cast<int>(player.get_x()) - static_cast<int>(this->pos_x);
        const int player_dy = static_cast<int>(player.get_y()) - static_cast<int>(this->pos_y);
        const int distance = std::max(std::abs(player_dx), std::abs(player_dy));
        if (distance < best_distance) {
            best_distance = distance;
            target = &player;
            dx = player_dx;
            dy = player_dy;
        }
    }
    if (target == nullptr) {
        return;
    }

    // Cuerpo a cuerpo: golpea a quien esté en las 8 vecinas.
    if (std::abs(dx) <= 1 && std::abs(dy) <= 1) {
        w.damage_player(target->get_id(), this->damage);
        return;
    }

    // Persigue un paso por tick: primero el eje con más distancia y, si está
    // bloqueado, intenta el otro para no quedarse clavado contra una pared.
    Direction first = std::abs(dx) > std::abs(dy) ? (dx > 0 ? Direction::RIGHT : Direction::LEFT) :
                                                    (dy > 0 ? Direction::DOWN : Direction::UP);
    Direction second = std::abs(dx) > std::abs(dy) ? (dy > 0 ? Direction::DOWN : Direction::UP) :
                                                     (dx > 0 ? Direction::RIGHT : Direction::LEFT);
    if (!this->move(w, first)) {
        this->move(w, second);
    }
}
