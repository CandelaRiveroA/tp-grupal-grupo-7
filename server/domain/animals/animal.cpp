#include "animal.h"

// Inicializamos la posición en IEntity y los días en 0
Animal::Animal(uint16_t x, uint16_t y) : IEntity(x, y), days_passed(0) {}
