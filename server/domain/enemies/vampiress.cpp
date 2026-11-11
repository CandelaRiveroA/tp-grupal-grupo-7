#include "vampiress.h"

#include <cstdint>

// Placeholders de balance (dificultad alta, variante rápida). Ajustables.
#define kHealth 20
// Con kMaxHealth = 100, 8 de daño mata al jugador en 13 golpes.
#define kDamage 8
#define kGoldDropPercent 40
#define kSpeed 2

Vampiress::Vampiress(uint16_t x, uint16_t y):
        Vampire(x, y, kHealth, kDamage, kGoldDropPercent, kSpeed) {}

void Vampiress::enter_frenzy() { this->speed_frenzy = 2; }
