#include "orc.h"

#include <cstdint>

#include "../map/world.h"

// Placeholders de balance (dificultad media). Ajustables.
#define kHealth 15
// Con kMaxHealth = 100, 10 de daño mata al jugador en 10 golpes.
#define kDamage 10
#define kGoldDropPercent 30
// Particularidad del orco: baja a propósito porque no revive.
#define kSilverDropPercent 20
// Precio del orbe de plata: el mismo que la piedra de plata.
#define kSilverOrbPrice 250

Orc::Orc(uint16_t x, uint16_t y): Orc(x, y, kSilverDropPercent) {}

Orc::Orc(uint16_t x, uint16_t y, uint8_t silver_drop_percent):
        Enemy(x, y, kHealth, kDamage, kGoldDropPercent), silver_drop_percent(silver_drop_percent) {}

std::string Orc::kind_name() const { return "orc"; }

void Orc::on_death(World& w) {
    this->drop_orb(w, ItemID::SILVER_ORB, kSilverOrbPrice, this->silver_drop_percent);
}
