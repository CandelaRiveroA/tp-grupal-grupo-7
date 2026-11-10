#include "player.h"

Player::Player(uint8_t id, uint16_t x, uint16_t y):
        IEntity(x, y), player_id(id), facing(Direction::DOWN), selected_slot(0), backpack(9) {
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
}
uint8_t Player::get_id() const { return this->player_id; }

Direction Player::get_facing() const { return this->facing; }

const CharacterAppearance& Player::get_appearance() const { return this->appearance; }

const Inventory& Player::get_inventory() const { return this->backpack; }

void Player::set_facing(Direction dir) { this->facing = dir; }

void Player::set_appearance(const CharacterAppearance& new_appearance) {
    this->appearance = new_appearance;
}

// Calcula la posición de la casilla objetivo según la dirección en la que el jugador está mirando.
std::pair<uint16_t, uint16_t> Player::get_target_tile() const {
    uint16_t target_x = this->pos_x;
    uint16_t target_y = this->pos_y;

    switch (this->facing) {
        case Direction::UP:
            if (target_y > 0)
                target_y--;
            break;
        case Direction::DOWN:
            target_y++;
            break;
        case Direction::LEFT:
            if (target_x > 0)
                target_x--;
            break;
        case Direction::RIGHT:
            target_x++;
            break;
    }

    return {target_x, target_y};
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

bool Player::is_walkable() const { return true; }

void Player::interact(Player& /*p*/, ToolType /*tool*/, World& /*w*/) {}

void Player::update(World& /*w*/) {}