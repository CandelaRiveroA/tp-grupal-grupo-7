#include "skeleton.h"

#include <cstdint>

#include "../map/world.h"

// Placeholders de balance (dificultad baja). Ajustables.
#define kHealth 10
// Con kMaxHealth = 100, 5 de daño mata al jugador en 20 golpes.
#define kDamage 5
// 15 ticks de CreatureTick (~4 s c/u) ≈ 60 s hasta el revive.
#define kReviveTicks 15
// Bajo a propósito: revive, así que con más probabilidad sería granja de oro.
#define kGoldDropPercent 20

Skeleton::Skeleton(uint16_t x, uint16_t y): Enemy(x, y, kHealth, kDamage, kGoldDropPercent) {}

void Skeleton::on_death(World& /*w*/) {
    // Particularidad del esqueleto: no se retira del mundo, revive con vida
    // llena cuando update() se acaba el contador.
    this->ticks_to_revive = kReviveTicks;
}

void Skeleton::behave(World& w) { this->chase_player(w); }
