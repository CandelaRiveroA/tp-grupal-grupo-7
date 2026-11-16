#include "skull.h"

#include <cstdint>
#include <cstdlib>

#include "../map/world.h"

// Placeholders de balance. Ajustables.
// Vida cosmética: take_damage nunca la baja.
#define kHealth 50
// Rango de aggro y de llamas: 3 (y no 5) para que el jugador pueda huir.
#define kAggroRange 7
// Distancia a la que se queda a tiro (celdas, Chebyshev): más cerca para que
// la llama casi no encuentre paredes en el camino, sin encimarse al jugador
// para que todavía pueda esquivarla.
#define kShotDistance 5
#define kBlueFlameDamage 5
// El doble que la azul.
#define kGreenFlameDamage 10
// La verde es la mitad de rápida: una cada 2 ticks.
#define kGreenFlameEveryTicks 2
// Vocabulario de enemies.toml (el cliente lo dibuja como SkullFire).
#define kBlueFlameKind "skull_fire"
#define kGreenFlameKind "skull_fire_green"

Skull::Skull(uint16_t x, uint16_t y): Enemy(x, y, kHealth, kBlueFlameDamage, 0), flame_tick(0) {}

bool Skull::take_damage(uint8_t /*incoming*/) { return false; }

std::string Skull::kind_name() const { return "skull"; }

void Skull::on_death(World& /*w*/) {}

bool Skull::pursue(World& w) {
    // Limpia la acción (y baja el cooldown) para rearmar el flanco de
    // animación del cliente, como antes.
    const bool rearm = this->pursue_steps(w, 0);
    const Player* target = this->find_nearest_player(w, kAggroRange);
    if (target == nullptr) {
        return rearm;
    }
    const int dx = static_cast<int>(target->get_x()) - static_cast<int>(this->pos_x);
    const int dy = static_cast<int>(target->get_y()) - static_cast<int>(this->pos_y);
    // Ya está a tiro: no se le encima.
    if (std::abs(dx) <= kShotDistance && std::abs(dy) <= kShotDistance) {
        return rearm;
    }
    return this->step_toward(w, *target) || rearm;
}

void Skull::behave(World& w) {
    ++this->flame_tick;
    const Player* target = this->find_nearest_player(w, kAggroRange);
    if (target == nullptr) {
        return;
    }
    this->action = EnemyAction::Attack;
    // La llama viaja hasta la celda que ocupaba el jugador al lanzar: ahí
    // pega al contacto y se apaga (esquivable moviéndose, y contra paredes
    // muere en el intento en lugar de atravesarlas).
    w.spawn_flame(this->pos_x, this->pos_y, target->get_x(), target->get_y(), kBlueFlameDamage,
                  kBlueFlameKind);
    if (this->flame_tick % kGreenFlameEveryTicks == 0) {
        w.spawn_flame(this->pos_x, this->pos_y, target->get_x(), target->get_y(),
                      kGreenFlameDamage, kGreenFlameKind);
    }
}
