#include "animal.h"

#include <random>
#include <utility>

#include "../map/world.h"

// shortcut: id por contador global de la partida (uint8, se repite tras 255 animales),
// upgrade: creo que se tenia que manejar por nombre y no id(?).
static uint8_t next_animal_id = 1;

// Velocidad del deambular (en ticks de CreatureTick = 1 s): mantiene un rumbo
// entre kWanderTurnMin y kWanderTurnMax, y se queda quieto entre
// kWanderPauseMin y kWanderPauseMax antes de arrancar el próximo tramo.
// Ajustable según la velocidad que se quiera.
#define kWanderTurnMin 2
#define kWanderTurnMax 5
#define kWanderPauseMin 1
#define kWanderPauseMax 3

// Un solo generador para todos los animales, sembrado al arrancar.
static std::mt19937& rng() {
    static std::mt19937 gen(std::random_device{}());
    return gen;
}

static uint8_t rand_ticks(int min, int max) {
    return static_cast<uint8_t>(std::uniform_int_distribution<int>(min, max)(rng()));
}

static Direction random_direction() {
    return static_cast<Direction>(std::uniform_int_distribution<int>(0, 3)(rng()));
}

// Inicializamos la posición en IEntity y los días en 0.
// La pausa inicial aleatoria desfasa a los animales entre sí para que no
// caminen todos en el mismo tick.
Animal::Animal(uint16_t x, uint16_t y):
        IEntity(x, y),
        days_passed(0),
        id(next_animal_id++),
        wander_dir(Direction::UP),
        ticks_to_turn(0),
        ticks_to_pause(rand_ticks(0, kWanderPauseMax)) {}

uint8_t Animal::get_id() const { return id; }

std::optional<AnimalSnapshot> Animal::animal_snapshot() const {
    return AnimalSnapshot{id, kind_name(), false, wander_dir};
}

bool Animal::try_hand_action(Player& /*p*/, World& /*w*/) { return false; }

void Animal::wander(World& w) {
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

    // Si el destino está bloqueado o es el borde, se reintenta en el mismo rumbo
    // hasta que toque cambiar de dirección.
    this->move(w, this->wander_dir);
}

bool Animal::move(World& w, Direction dir) {
    const auto [target_x, target_y] = IMovable::next_tile(this->pos_x, this->pos_y, dir);
    if (target_x == this->pos_x && target_y == this->pos_y) {
        return false;
    }
    w.move_entity(this, target_x, target_y);
    return this->pos_x == target_x && this->pos_y == target_y;
}
