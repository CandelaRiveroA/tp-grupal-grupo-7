#ifndef CELL_H
#define CELL_H

#include <memory>
#include "../entities/i_entity.h"

struct Cell {
    bool is_wall; 
    std::unique_ptr<IEntity> entity;

    Cell();
    Cell(const Cell&) = delete;
    Cell& operator=(const Cell&) = delete;
    Cell(Cell&&) = default;
    Cell& operator=(Cell&&) = default;

    bool is_empty() const;
};

#endif // CELL_H
