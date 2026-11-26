#include "player.h"
#include <cstdlib>

#include "i_movable.h"
#include "../inventory/tool.h"
#include "../inventory/product.h"
#include "../map/world.h"

// Vida máxima del jugador. La baja enemiga es 5-10 por golpe (kDamage de
// esqueleto y orco), así que un esqueleto mata en 20 golpes y un orco en 10.
#define kMaxHealth 100
// Escala de los porcentajes (0 a 100).
#define kPercentMax 100
// Por debajo de este porcentaje de la vida máxima el jugador queda gravemente herido.
#define kGravelyWoundedPercent 20
#define kNormalMovementSteps 2
#define kWoundedMovementSteps 1
#define kNormalSwordDamage 2
#define kWoundedSwordDamage 1

Player::Player(uint8_t id, uint16_t x, uint16_t y, SceneId initial_scene):
        Player(id, x, y, initial_scene, initial_scene, x, y) {}

Player::Player(uint8_t id, uint16_t x, uint16_t y, SceneId initial_scene,
               SceneId home_scene, uint16_t home_x, uint16_t home_y)
        : IEntity(x, y), player_id(id), current_scene(initial_scene), home_scene(home_scene),
          home_x(home_x), home_y(home_y), facing(Direction::DOWN), selected_slot(0), backpack(9),
          energy(10000), max_energy(10000), hp(kMaxHealth), max_hp(kMaxHealth),
          walking(false), died_in_combat(false), fainted_by_timeout(false),
          fainted_by_energy(false) {
    // Cargamos el inventario inicial del jugador con las herramientas básicas
    this->backpack.add_item(std::make_unique<Tool>(ItemID::PICKAXE));
    this->backpack.add_item(std::make_unique<Tool>(ItemID::SHOVEL));
    this->backpack.add_item(std::make_unique<Tool>(ItemID::WATERING_CAN));
    this->backpack.add_item(std::make_unique<Tool>(ItemID::SWORD));
        // shortcut: una semilla de cada cultivo fija para probar el ciclo; subir = que
    // se compren o se cosechen semillas (hoy no hay tienda).
    this->backpack.add_item(std::make_unique<Product>(ItemID::STRAWBERRY_SEED, 20, Quality::BASE));
    this->backpack.add_item(std::make_unique<Product>(ItemID::ONION_SEED, 20, Quality::BASE));
    this->backpack.add_item(std::make_unique<Product>(ItemID::POTATO_SEED, 20, Quality::BASE));
    this->backpack.add_item(std::make_unique<Product>(ItemID::PUMPKIN_SEED, 20, Quality::BASE));
    this->gold = std::rand() % 101;
}
uint8_t Player::get_id() const { return this->player_id; }

SceneId Player::get_scene() const { return this->current_scene; }
SceneId Player::get_home_scene() const { return this->home_scene; }
uint16_t Player::get_home_x() const { return this->home_x; }
uint16_t Player::get_home_y() const { return this->home_y; }
void Player::set_home(SceneId scene, uint16_t x, uint16_t y) {
    this->home_scene = scene;
    this->home_x = x;
    this->home_y = y;
}

Direction Player::get_facing() const { return this->facing; }

const CharacterAppearance& Player::get_appearance() const { return this->appearance; }

const Inventory& Player::get_inventory() const { return this->backpack; }

uint16_t Player::get_health() const { return this->hp; }
void Player::set_health(uint16_t hp) { this->hp = hp; }
uint16_t Player::get_gold() const { return this->gold; }
void Player::set_gold(uint16_t g) { this->gold = g; }
uint16_t Player::get_max_health() const { return this->max_hp; }
uint16_t Player::get_energy() const { return this->energy; }
void Player::set_energy(uint16_t e) { this->energy = e; }
uint16_t Player::get_max_energy() const { return this->max_energy; }
void Player::set_max_health(uint16_t value) { this->max_hp = value; }
void Player::set_max_energy(uint16_t value) { this->max_energy = value; }

void Player::set_scene(SceneId scene) { this->current_scene = scene; }
void Player::add_gold(uint16_t amount) { this->gold += amount; }
bool Player::try_remove_gold(uint16_t amount) {
    if (this->gold >= amount) {
        this->gold -= amount;
        return true;
    }
    return false;
}
bool Player::diedInCombat() const { return this->died_in_combat; }
bool Player::faintedByTimeout() const { return this->fainted_by_timeout; }
bool Player::faintedByEnergy() const { return this->fainted_by_energy; }
void Player::setDiedInCombat(bool v) { this->died_in_combat = v; }
void Player::setFaintedByTimeout(bool v) { this->fainted_by_timeout = v; }
void Player::setFaintedByEnergy(bool v) { this->fainted_by_energy = v; }

bool Player::is_gravely_wounded() const {
    return this->hp * kPercentMax < this->max_hp * kGravelyWoundedPercent;
}

uint8_t Player::get_movement_steps() const {
    if (this->is_gravely_wounded() || this->is_exhausted() || this->walking) {
        return kWoundedMovementSteps;
    }
    return kNormalMovementSteps;
}

void Player::toggle_walking() { this->walking = !this->walking; }

void Player::set_walking(bool active) { this->walking = active; }

bool Player::is_walking() const { return this->walking; }

uint8_t Player::get_sword_damage() const {
    return this->is_gravely_wounded() ? kWoundedSwordDamage : kNormalSwordDamage;
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

bool Player::can_pick_up_item(const Item& item) const {
    return backpack.can_add_item(item);
}

bool Player::pick_up_item(std::unique_ptr<Item> item) {
    return this->backpack.add_item(std::move(item));
}

std::vector<std::unique_ptr<Item>> Player::extract_items(ItemID id, uint16_t qty) {
    return this->backpack.extract_items(id, qty);
}

std::vector<std::unique_ptr<Item>> Player::extract_items_with_min_quality(
        ItemID id, Quality minimum_quality, uint16_t qty) {
    return this->backpack.extract_items_with_min_quality(id, minimum_quality, qty);
}

bool Player::can_extract_items(ItemID id, uint16_t qty) const {
    return this->backpack.can_extract_items(id, qty);
}

bool Player::can_extract_items_with_min_quality(
        ItemID id, Quality minimum_quality, uint16_t qty) const {
    return this->backpack.can_extract_items_with_min_quality(id, minimum_quality, qty);
}

void Player::consume_energy(uint16_t amount) {
    if (this->energy >= amount) {
        this->energy -= amount;
    } else {
        this->energy = 0;
        // TODO: Disparar evento de desmayo automático[cite: 1]
    }
}

bool Player::is_exhausted() const {
    return this->max_energy == 0 ||
           static_cast<uint32_t>(this->energy) * 100u <=
                   static_cast<uint32_t>(this->max_energy) * 10u;
}

bool Player::is_energy_depleted() const {
    return this->energy == 0;
}

bool Player::is_walkable() const { return true; }

void Player::interact(Player& p, ToolType tool, World& w) {
    if (tool != ToolType::HAND) return;
    const Item* item_to_give = p.peek_equipped_item();
    if (!item_to_give || !item_to_give->can_be_traded()) {
        return; 
    }
    w.notify_gift_request(p.get_id(), this->get_id(), item_to_give->get_id());   
}

bool Player::finalize_gift_reception(Player& receiver) {
    const Item* equipped = this->peek_equipped_item();
    
    if (!equipped || !equipped->can_be_traded()) {
        return false; 
    }
    
    std::unique_ptr<Item> gift = equipped->clone();
    bool success = receiver.pick_up_item(std::move(gift));

    if (success) {
        this->consume_equipped_item();
        return true;
    }

    return false;
}

void Player::update(World& /*w*/) {}