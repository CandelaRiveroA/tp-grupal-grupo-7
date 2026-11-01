#include "cell.h"

Cell::Cell() : is_wall(false) {}

bool Cell::is_empty() const {
    return this->entities.empty();
}
