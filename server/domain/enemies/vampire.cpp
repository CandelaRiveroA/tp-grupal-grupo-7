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
// Multiplicador que aplica el frenesí a la habilidad especial.
#define kFrenzyMultiplier 2

Vampire::Vampire(uint16_t x, uint16_t y):
        Vampire(x, y, kHealth, kDamage, kGoldDropPercent, kSpeed) {}

Vampire::~Vampire() {
    if (this->partner != nullptr && this->partner->partner == this) {
        this->partner->partner = nullptr;
    }
}

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

void Vampire::enter_frenzy() { this->attack_frenzy = kFrenzyMultiplier; }

bool Vampire::try_consume_for_revive(World& w) {
    return w.consume_blood_puddle();
}

std::string Vampire::kind_name() const { return "vampire"; }

void Vampire::on_death(World& w) {
    // Revive solo si queda sangre; el charco se consume al revivir (no al
    // morir), así el cliente ve la eliminación cuando el vampiro vuelve.
    if (w.has_blood_puddle()) {
        this->ticks_to_revive = kReviveTicks;
    }
    // El que muere enfurece a su pareja aunque él no reviva.
    if (this->partner != nullptr) {
        this->partner->enter_frenzy();
    }
}

bool Vampire::pursue(World& w) {
    // Un paso de persecución por punto de speed por tick (400 ms).
    return this->pursue_steps(w, this->speed * this->speed_frenzy);
}
