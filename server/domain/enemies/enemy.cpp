#include "enemy.h"

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <utility>  // cpplint: lo pide por el método move (no usa std::move)

#include "../entities/dropped_item.h"
#include "../inventory/product.h"
#include "../map/world.h"

// Precio del orbe de oro: el mismo que la piedra de oro.
#define kGoldOrbPrice 750
// Radio de persecución cuerpo a cuerpo (celdas, distancia de Chebyshev).
#define kAggroRange 5
// Escala de los porcentajes de drop (0 a 100).
#define kPercentMax 100
// Cooldown de ataque en ticks de persecución (400 ms c/u = 4 s): el daño por
// segundo es el mismo que antes del reloj rápido y cada golpe cabe entero
// dentro de la ventana de animación (1,5 s) que abre el cliente.
#define kAttackCooldownTicks 10
// Velocidad del deambular (en ticks de CreatureTick): misma escala que Animal.
#define kWanderTurnMin 2
#define kWanderTurnMax 5
#define kWanderPauseMin 1
#define kWanderPauseMax 3

// Dirección al azar entre las 4 (mismo rango que usa Animal).
static Direction random_direction() {
    return static_cast<Direction>(std::rand() % 4);
}

// Duración al azar en [min, max].
static uint8_t rand_ticks(int min, int max) {
    return static_cast<uint8_t>(min + std::rand() % (max - min + 1));
}

Enemy::Enemy(uint16_t x, uint16_t y, uint8_t health, uint8_t damage, uint8_t gold_drop_percent):
        IEntity(x, y),
        health(health),
        max_health(health),
        damage(damage),
        ticks_to_revive(0),
        gold_drop_percent(gold_drop_percent),
        id(0),
        facing(Direction::DOWN),
        action(EnemyAction::Idle),
        attack_cooldown(0),
        wander_dir(Direction::UP),
        ticks_to_turn(0),
        ticks_to_pause(0),
        group_id(0),
        pair_id(0),
        room(0) {}

uint8_t Enemy::get_health() const { return this->health; }

void Enemy::set_id(uint16_t new_id) { this->id = new_id; }

void Enemy::set_group_id(uint16_t value) { this->group_id = value; }

void Enemy::set_pair_id(uint16_t value) { this->pair_id = value; }

void Enemy::set_room(uint16_t value) { this->room = value; }

std::optional<EnemySnapshot> Enemy::enemy_snapshot() const {
    return EnemySnapshot{this->id, this->kind_name(), this->facing, this->health, this->action};
}

uint8_t Enemy::get_damage() const { return this->damage; }

bool Enemy::take_damage(uint8_t incoming) {
    // Un muerto no recibe más golpes.
    if (this->health == 0) {
        return false;
    }
    this->action = EnemyAction::Damage;
    this->health = incoming >= this->health ? 0 : this->health - incoming;
    return this->health == 0;
}

bool Enemy::move(World& w, Direction dir) {
    this->facing = dir;
    const auto [target_x, target_y] = IMovable::next_tile(this->pos_x, this->pos_y, dir);
    if (target_x == this->pos_x && target_y == this->pos_y) {
        return false;
    }
    w.move_entity(this, target_x, target_y);
    return this->pos_x == target_x && this->pos_y == target_y;
}

// El cadáver no bloquea el paso.
bool Enemy::is_walkable() const { return this->health == 0; }

void Enemy::interact(Player& p, ToolType tool, World& w) {
    // Solo la espada hace daño; take_damage ignora los golpes de un muerto.
    if (tool != ToolType::SWORD) {
        return;
    }
    if (this->take_damage(p.get_sword_damage())) {
        w.record_enemy_killed(this->kind_name(), this->group_id, this->pair_id,
                              this->room == 0 ? w.get_current_room() : this->room);
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

bool Enemy::chase_player(World& w) {
    const Player* target = this->find_nearest_player(w, kAggroRange);
    if (target == nullptr) {
        return false;
    }
    const int dx = static_cast<int>(target->get_x()) - static_cast<int>(this->pos_x);
    const int dy = static_cast<int>(target->get_y()) - static_cast<int>(this->pos_y);

    // Cuerpo a cuerpo: golpea a quien esté en las 8 vecinas, pero como mucho
    // una vez cada kAttackCooldownTicks pasos (así el daño por segundo es el
    // mismo que antes del reloj rápido y la animación llega a verse completa).
    if (std::abs(dx) <= 1 && std::abs(dy) <= 1) {
        if (this->attack_cooldown > 0) {
            return false;
        }
        this->action = EnemyAction::Attack;
        w.damage_player(target->get_id(), this->get_damage());
        this->attack_cooldown = kAttackCooldownTicks;
        return true;
    }

    return this->step_toward(w, *target);
}

bool Enemy::step_toward(World& w, const Player& target) {
    const int dx = static_cast<int>(target.get_x()) - static_cast<int>(this->pos_x);
    const int dy = static_cast<int>(target.get_y()) - static_cast<int>(this->pos_y);
    const Direction first = std::abs(dx) > std::abs(dy) ? (dx > 0 ? Direction::RIGHT : Direction::LEFT) :
                                                          (dy > 0 ? Direction::DOWN : Direction::UP);
    const Direction second = std::abs(dx) > std::abs(dy) ? (dy > 0 ? Direction::DOWN : Direction::UP) :
                                                           (dx > 0 ? Direction::RIGHT : Direction::LEFT);
    if (this->move(w, first)) {
        return true;
    }
    return this->move(w, second);
}

bool Enemy::pursue(World& w) { return this->pursue_steps(w, 1); }

bool Enemy::pursue_steps(World& w, uint8_t steps) {
    // La acción dura un tick de wire: se limpia acá para que el próximo golpe
    // llegue como flanco Idle -> Attack y el cliente abra su ventana.
    const EnemyAction previous = this->action;
    this->action = EnemyAction::Idle;
    if (this->attack_cooldown > 0) {
        --this->attack_cooldown;
    }
    bool changed = previous != EnemyAction::Idle;
    for (uint8_t step = 0; step < steps; ++step) {
        changed = this->chase_player(w) || changed;
    }
    return changed;
}

void Enemy::wander(World& w) {
    // Con un jugador en rango de aggro no deambula: lo persigue el reloj rápido.
    if (this->find_nearest_player(w, kAggroRange) != nullptr) {
        return;
    }
    // Todavía no le toca moverse: se queda quieto unos ticks.
    if (this->ticks_to_pause > 0) {
        --this->ticks_to_pause;
        return;
    }
    if (this->ticks_to_turn == 0) {
        this->wander_dir = random_direction();
        this->ticks_to_turn = rand_ticks(kWanderTurnMin, kWanderTurnMax);
        this->ticks_to_pause = rand_ticks(kWanderPauseMin, kWanderPauseMax);
    }
    --this->ticks_to_turn;
    // Si el destino está bloqueado o es el borde, se reintenta en el mismo rumbo.
    this->move(w, this->wander_dir);
}

void Enemy::behave(World& w) { this->wander(w); }

void Enemy::update(World& w) {
    if (this->health == 0) {
        if (this->ticks_to_revive > 0) {
            --this->ticks_to_revive;
            if (this->ticks_to_revive == 0) {
                // Consume al revivir: si ya no queda sangre, se queda muerto.
                if (this->try_consume_for_revive(w)) {
                    this->health = this->max_health;  // Revive con vida llena.
                    this->action = EnemyAction::Idle;
                }
            }
        }
        return;
    }
    // La acción dura un tick: se limpia antes de que la especie decida la nueva.
    this->action = EnemyAction::Idle;
    this->behave(w);
}
