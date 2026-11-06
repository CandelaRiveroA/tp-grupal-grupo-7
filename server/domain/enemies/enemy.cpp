#include "enemy.h"

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

Enemy::Enemy(uint16_t x, uint16_t y, uint8_t health, uint8_t damage, uint8_t gold_drop_percent):
        IEntity(x, y),
        health(health),
        max_health(health),
        damage(damage),
        ticks_to_revive(0),
        gold_drop_percent(gold_drop_percent) {}

uint8_t Enemy::get_health() const { return this->health; }

bool Enemy::take_damage(uint8_t incoming) {
    // Un muerto no recibe más golpes.
    if (this->health == 0) {
        return false;
    }
    this->health = incoming >= this->health ? 0 : this->health - incoming;
    return this->health == 0;
}

bool Enemy::move(World& w, Direction dir) {
    uint16_t target_x = this->pos_x;
    uint16_t target_y = this->pos_y;

    switch (dir) {
        case Direction::UP:
            if (target_y > 0)
                --target_y;
            break;
        case Direction::DOWN:
            ++target_y;
            break;
        case Direction::LEFT:
            if (target_x > 0)
                --target_x;
            break;
        case Direction::RIGHT:
            ++target_x;
            break;
    }

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
        this->drop_gold(w);
        this->on_death(w);
    }
}

void Enemy::drop_gold(World& w) {
    if (std::rand() % 100 >= this->gold_drop_percent) {
        return;
    }
    auto orb = std::make_unique<Product>(ItemID::GOLD_ORB, kGoldOrbPrice, Quality::BASE);
    w.set_entity_at(this->pos_x, this->pos_y,
                    std::make_unique<DroppedItem>(std::move(orb), this->pos_x, this->pos_y));
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
