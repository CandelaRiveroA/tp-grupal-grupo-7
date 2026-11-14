#include "player.h"
#include <cstdlib>

#include "i_movable.h"

// Vida máxima del jugador. La baja enemiga es 5-10 por golpe (kDamage de
// esqueleto y orco), así que un esqueleto mata en 20 golpes y un orco en 10.
#define kMaxHealth 100
// Escala de los porcentajes (0 a 100).
#define kPercentMax 100
// Por debajo de este porcentaje de la vida máxima el jugador queda gravemente herido.
#define kGravelyWoundedPercent 20

Player::Player(uint8_t id, uint16_t x, uint16_t y)
        : IEntity(x, y), player_id(id), facing(Direction::DOWN), selected_slot(0), backpack(9),
          energy(100.0f), max_energy(100.0f), hp(kMaxHealth), max_hp(kMaxHealth) {
    // Cargamos el inventario inicial del jugador con las herramientas básicas
    this->backpack.add_item(std::make_unique<Item>(ItemID::PICKAXE, false));
    this->backpack.add_item(std::make_unique<Item>(ItemID::SHOVEL, false));
    this->backpack.add_item(std::make_unique<Item>(ItemID::WATERING_CAN, false));
    this->backpack.add_item(std::make_unique<Item>(ItemID::SWORD, false));
        // shortcut: una semilla de cada cultivo fija para probar el ciclo; subir = que
    // se compren o se cosechen semillas (hoy no hay tienda).
    this->backpack.add_item(std::make_unique<Item>(ItemID::STRAWBERRY_SEED, false));
    this->backpack.add_item(std::make_unique<Item>(ItemID::ONION_SEED, false));
    this->backpack.add_item(std::make_unique<Item>(ItemID::POTATO_SEED, false));
    this->backpack.add_item(std::make_unique<Item>(ItemID::PUMPKIN_SEED, false));
    this->gold = std::rand() % 101;
}
uint8_t Player::get_id() const { return this->player_id; }

Direction Player::get_facing() const { return this->facing; }

const CharacterAppearance& Player::get_appearance() const { return this->appearance; }

const Inventory& Player::get_inventory() const { return this->backpack; }

uint16_t Player::get_health() const { return this->hp; }

bool Player::is_gravely_wounded() const {
    return this->hp * kPercentMax < this->max_hp * kGravelyWoundedPercent;
}

bool Player::take_damage(uint8_t incoming) {
    // shortcut: llegar a 0 vida no tiene consecuencia todavía (el jugador
    // sigue jugando); la muerte y el respawn quedan para cuando exista ese
    // sistema.
    if (this->hp == 0) {
        return false;  // ya estaba sin vida: este golpe no fue el que lo mató
    }
    this->hp = incoming >= this->hp ? 0 : this->hp - incoming;
    return this->hp == 0;
}

void Player::set_facing(Direction dir) { this->facing = dir; }

void Player::set_appearance(const CharacterAppearance& new_appearance) {
    this->appearance = new_appearance;
}

// Calcula la posición de la casilla objetivo según la dirección en la que el jugador está mirando.
std::pair<uint16_t, uint16_t> Player::get_target_tile() const {
    return IMovable::next_tile(this->pos_x, this->pos_y, this->facing);
}

void Player::select_slot(uint8_t slot_idx) {
    if (slot_idx < 9) {
        this->selected_slot = slot_idx;
    }
}

const Item* Player::peek_equipped_item() const {
    return this->backpack.peek_item(this->selected_slot);
}

void Player::consume_equipped_item() {
    this->backpack.consume_item(this->selected_slot);
}

ToolType Player::get_equipped_tool() const {
    // Si el slot está vacío, las manos están desarmadas
    if (this->backpack.is_slot_empty(this->selected_slot)) {
        return ToolType::HAND;
    }

    const Item* current_item = this->backpack.peek_item(this->selected_slot);
    if (!current_item) {
        return ToolType::HAND;
    }

    // TODO: Separar la validación de herramientas y productos en un método aparte para mantener la
    // claridad del código.

    // Mapeo seguro según el ItemID de las herramientas iniciales
    switch (current_item->get_id()) {
        case ItemID::PICKAXE:
            return ToolType::PICKAXE;
        case ItemID::SHOVEL:
            return ToolType::SHOVEL;
        case ItemID::WATERING_CAN:
            return ToolType::WATERING_CAN;
        case ItemID::SWORD:
            return ToolType::SWORD;
        default:
            return ToolType::HAND;  // Productos, minerales o semillas cuentan como HAND
    }
}

bool Player::pick_up_item(std::unique_ptr<Item> item) {
    return this->backpack.add_item(std::move(item));
}

void Player::consume_energy(float amount) {
    if (this->energy >= amount) {
        this->energy -= amount;
    } else {
        this->energy = 0.0f;
        // TODO: Disparar evento de desmayo automático[cite: 1]
    }
}

bool Player::is_exhausted() const {
    return (this->energy / this->max_energy) <= 0.10f;
}

bool Player::is_energy_depleted() const {
    return this->energy <= 0.0f;
}

bool Player::is_walkable() const { return true; }

void Player::interact(Player& /*p*/, ToolType /*tool*/, World& /*w*/) {}

void Player::update(World& /*w*/) {}