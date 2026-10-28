#include "cell.h"

Cell::Cell() : is_wall(false), entity(nullptr) {}

bool Cell::is_empty() const {
    return this->entity == nullptr;
}
