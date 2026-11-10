#include "bed.h"

#include "../inventory/item_types.h"
#include "../map/world.h"

// shortcut: model_id 0 = cama. Nadie lo lee todavía; cuando haya mesas y
// sillas hay que darle un modelo por mueble.
Bed::Bed(uint16_t x, uint16_t y): Furniture(x, y, ItemID::BED, 0) {}

bool Bed::is_walkable() const { return true; }

void Bed::on_use(Player& p, World& w) { w.record_sleep_request(p.get_id()); }
