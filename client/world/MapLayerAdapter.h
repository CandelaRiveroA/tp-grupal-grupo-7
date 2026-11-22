#pragma once

#include <string>

#include "world/SceneDescription.h"
#include "world_model/Map.h"

namespace world {

// Piso y caminos del editor como capas en grilla de la escena (una celda
// de arte por celda lógica), de abajo hacia arriba.
class MapLayerAdapter {
public:
	void add(const worldmodel::Map& map, const std::string& mapPath,
		 SceneDescription& scene) const;

private:
	// Una capa; si no tiene ningún tile no se agrega.
	void addLayer(const worldmodel::TileGrid& grid, const std::string& path,
		      const worldmodel::Map& map, SceneDescription& scene,
		      LayerPass pass = LayerPass::Background) const;
};

}
