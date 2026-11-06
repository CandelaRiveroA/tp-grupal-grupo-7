#ifndef DECORATION_ITEM_H
#define DECORATION_ITEM_H

#include <cstdint>

#include "item.h"

// Ítem que dropea un mueble al destruirlo. model_id distingue el modelo
// visual dentro de un mismo ItemID (hoy nadie lo lee: el cliente lo usará
// cuando dibuje la decoración).
class DecorationItem: public Item {
private:
    uint16_t model_id;

public:
    DecorationItem(ItemID id, uint16_t model_id);
    ~DecorationItem() override = default;
    DecorationItem(const DecorationItem&) = delete;
    DecorationItem& operator=(const DecorationItem&) = delete;
    DecorationItem(DecorationItem&&) = default;
    DecorationItem& operator=(DecorationItem&&) = default;

    uint16_t get_model_id() const;
};

#endif  // DECORATION_ITEM_H
