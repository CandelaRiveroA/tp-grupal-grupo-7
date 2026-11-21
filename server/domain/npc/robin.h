#ifndef ROBIN_H
#define ROBIN_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "merchant.h"

class Robin: public Merchant {
private:
    std::vector<std::pair<std::string, CatalogEntry>> full_pool;
    std::size_t current_offset;

public:
    Robin(uint16_t x, uint16_t y);
    ~Robin() override = default;
    // Para cambiar los items que vende Robin cada día,
    // se hace un shuffle de la lista completa
    // TODO: --
    void on_new_day() override;
};

#endif  // ROBIN_H
