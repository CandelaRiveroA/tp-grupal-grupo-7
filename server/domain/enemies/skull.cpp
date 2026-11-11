#include "skull.h"

#include <cstdint>

#include "../map/world.h"

// Placeholders de balance. Ajustables.
// Vida cosmética: take_damage nunca la baja.
#define kHealth 50
// Rango de aggro y de llamas: 3 (y no 5) para que el jugador pueda huir.
#define kAggroRange 3
#define kBlueFlameDamage 5
// El doble que la azul.
#define kGreenFlameDamage 10
// La verde es la mitad de rápida: una cada 2 ticks.
#define kGreenFlameEveryTicks 2

Skull::Skull(uint16_t x, uint16_t y): Enemy(x, y, kHealth, kBlueFlameDamage, 0), flame_tick(0) {}

bool Skull::take_damage(uint8_t /*incoming*/) { return false; }

void Skull::on_death(World& /*w*/) {}

void Skull::behave(World& w) {
    ++this->flame_tick;
    const Player* target = this->find_nearest_player(w, kAggroRange);
    if (target == nullptr) {
        return;
    }
    // shortcut: sin línea de vista, la llama atraviesa paredes; cuando haya
    // visibilidad en el mapa, filtrar el objetivo por ella.
    w.damage_player(target->get_id(), kBlueFlameDamage);
    if (this->flame_tick % kGreenFlameEveryTicks == 0) {
        w.damage_player(target->get_id(), kGreenFlameDamage);
    }
}
