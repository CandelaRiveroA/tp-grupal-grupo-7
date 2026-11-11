#include "vampire.h"

#include <cstdint>

#include "../map/world.h"

// Placeholders de balance (dificultad alta). Ajustables.
#define kHealth 30
// Con kMaxHealth = 100, 15 de daño mata al jugador en 7 golpes (3 en frenesí).
#define kDamage 15
#define kGoldDropPercent 40
#define kSpeed 1
// 10 ticks de CreatureTick (~4 s c/u) ≈ 40 s hasta el revive.
#define kReviveTicks 10

Vampire::Vampire(uint16_t x, uint16_t y):
        Vampire(x, y, kHealth, kDamage, kGoldDropPercent, kSpeed) {}

Vampire::Vampire(uint16_t x, uint16_t y, uint8_t health, uint8_t damage,
                 uint8_t gold_drop_percent, uint8_t speed):
        Enemy(x, y, health, damage, gold_drop_percent),
        speed(speed),
        attack_frenzy(1),
        speed_frenzy(1),
        partner(nullptr) {}

void Vampire::set_partner(Vampire* other) { this->partner = other; }

uint8_t Vampire::get_damage() const {
    return static_cast<uint8_t>(this->damage * this->attack_frenzy);
}

void Vampire::enter_frenzy() { this->attack_frenzy = 2; }

void Vampire::on_death(World& w) {
    // Particularidad: revive solo si quedaba sangre; consume un charco por
    // muerte. Sin sangre se queda muerto.
    if (w.consume_blood_puddle()) {
        this->ticks_to_revive = kReviveTicks;
    }
    // El que muere enfurece a su pareja aunque él no reviva.
    if (this->partner != nullptr) {
        this->partner->enter_frenzy();
    }
}

void Vampire::behave(World& w) {
    // Una acción (mover o pegar) por cada punto de speed por tick.
    for (uint8_t i = 0; i < this->speed * this->speed_frenzy; ++i) {
        this->chase_player(w);
    }
}
