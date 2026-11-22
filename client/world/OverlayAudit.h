#pragma once

#include <vector>

#include "world/ConfigError.h"
#include "world/WorldOverlay.h"
#include "world_model/World.h"

namespace world {

// Detecta las entradas del overlay que no coinciden con nada del mundo
// (un tile mal escrito, un mapa que ya no existe). No son fatales: el
// overlay es opcional y lo que no coincide simplemente no se aplica. Se
// devuelven como datos (kind + path) para que UN solo lugar los muestre.
class OverlayAudit {
public:
	std::vector<ConfigError> unmatched(const worldmodel::World& world,
					   const WorldOverlay& overlay) const;
};

}
