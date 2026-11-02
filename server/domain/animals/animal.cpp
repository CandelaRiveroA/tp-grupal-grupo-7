#include "animal.h"

// shortcut: id por contador global de la partida (uint8, se repite tras 255 animales),
// upgrade: creo que se tenia que manejar por nombre y no id(?).
static uint8_t next_animal_id = 1;

// Inicializamos la posición en IEntity y los días en 0
Animal::Animal(uint16_t x, uint16_t y): IEntity(x, y), days_passed(0), id(next_animal_id++) {}

uint8_t Animal::get_id() const { return id; }

std::optional<AnimalSnapshot> Animal::animal_snapshot() const {
    return AnimalSnapshot{id, kind_name(), false};
}
