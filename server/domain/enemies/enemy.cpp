#include "enemy.h"

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <utility>  // cpplint: lo pide por el método move (no usa std::move)

#include "../entities/dropped_item.h"
#include "../inventory/product.h"
#include "../map/world.h"

// Daño de la espada por golpe. shortcut: placeholder; cuando exista daño por
// ítem debería salir del inventario, no de una constante.
#define kSwordDamage 1
// Precio del orbe de oro: el mismo que la piedra de oro.
#define kGoldOrbPrice 750
// Radio de persecución cuerpo a cuerpo (celdas, distancia de Chebyshev).
#define kAggroRange 5
// Escala de los porcentajes de drop (0 a 100).
#define kPercentMax 100

Enemy::Enemy(uint16_t x, uint16_t y, uint8_t health, uint8_t damage, uint8_t gold_drop_percent):
        IEntity(x, y),
        health(health),
        max_health(health),
        damage(damage),
        ticks_to_revive(0),
        gold_drop_percent(gold_drop_percent) {}

uint8_t Enemy::get_health() const { return this->health; }

uint8_t Enemy::get_damage() const { return this->damage; }

bool Enemy::take_damage(uint8_t incoming) {
    // Un muerto no recibe más golpes.
    if (this->health == 0) {
        return false;
    }
    this->health = incoming >= this->health ? 0 : this->health - incoming;
    return this->health == 0;
}

bool Enemy::move(World& w, Direction dir) {
    const auto [target_x, target_y] = IMovable::next_tile(this->pos_x, this->pos_y, dir);
    if (target_x == this->pos_x && target_y == this->pos_y) {
        return false;
    }
    w.move_entity(this, target_x, target_y);
    return this->pos_x == target_x && this->pos_y == target_y;
}

// El cadáver no bloquea el paso.
bool Enemy::is_walkable() const { return this->health == 0; }

void Enemy::interact(Player& /*p*/, ToolType tool, World& w) {
    // Solo la espada hace daño; take_damage ignora los golpes de un muerto.
    if (tool != ToolType::SWORD) {
        return;
    }
    if (this->take_damage(kSwordDamage)) {
        this->drop_orb(w, ItemID::GOLD_ORB, kGoldOrbPrice, this->gold_drop_percent);
        this->on_death(w);
    }
}

void Enemy::drop_orb(World& w, ItemID item, uint16_t price, uint8_t percent) {
    if (std::rand() % kPercentMax >= percent) {
        return;
    }
    auto orb = std::make_unique<Product>(item, price, Quality::BASE);
    w.set_entity_at(this->pos_x, this->pos_y,
                    std::make_unique<DroppedItem>(std::move(orb), this->pos_x, this->pos_y));
}

const Player* Enemy::find_nearest_player(World& w, int range) const {
    const Player* target = nullptr;
    int best_distance = range + 1;
    for (const uint8_t id: w.get_player_ids()) {
        const Player& player = w.get_player(id);
        if (player.get_health() == 0) {
            continue;
        }
        const int player_dx = static_cast<int>(player.get_x()) - static_cast<int>(this->pos_x);
        const int player_dy = static_cast<int>(player.get_y()) - static_cast<int>(this->pos_y);
        const int distance = std::max(std::abs(player_dx), std::abs(player_dy));
        if (distance < best_distance) {
            best_distance = distance;
            target = &player;
        }
    }
    return target;
}

void Enemy::chase_player(World& w) {
    const Player* target = this->find_nearest_player(w, kAggroRange);
    if (target == nullptr) {
        return;
    }
    const int dx = static_cast<int>(target->get_x()) - static_cast<int>(this->pos_x);
    const int dy = static_cast<int>(target->get_y()) - static_cast<int>(this->pos_y);

    // Cuerpo a cuerpo: golpea a quien esté en las 8 vecinas.
    if (std::abs(dx) <= 1 && std::abs(dy) <= 1) {
        w.damage_player(target->get_id(), this->get_damage());
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

void Enemy::update(World& w) {
    if (this->health == 0) {
        if (this->ticks_to_revive > 0) {
            --this->ticks_to_revive;
            if (this->ticks_to_revive == 0) {
                this->health = this->max_health;  // Revive con vida llena.
            }
        }
        return;
    }
    this->behave(w);
}
