#pragma once

#include "world/SceneDescription.h"
#include "world/WorldOverlay.h"
#include "world_model/Map.h"

namespace world {

// Markers y portales del mapa como datos de la escena, y la celda por
// defecto de los NPC que el overlay ata a un marker.
class MapAnchorAdapter {
public:
	// El overlay es PRESTADO: debe vivir más que el adaptador.
	explicit MapAnchorAdapter(const WorldOverlay& overlay);

	void add(const worldmodel::Map& map, SceneDescription& scene) const;

private:
	const WorldOverlay& overlay_;
};

}
