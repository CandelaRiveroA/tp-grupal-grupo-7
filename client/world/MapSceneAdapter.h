#pragma once

#include <string>

#include "world/SceneDescription.h"
#include "world_model/World.h"

namespace world {

// Traduce un mapa del editor (common/world_model) al modelo de escena
// del cliente. Es el único que conoce las dos formas.
//
// Contrato de coherencia con la simulación: UNA celda del editor es UNA
// celda lógica de la escena (`cellPixelSize = tile_size * scale`), con
// una sola escala para todo el mapa. Un objeto de `size = [w, h]`
// ocupa exactamente w*h celdas (`span = size`), se dibuje del tamaño
// que se dibuje. Si una hoja no encaja exacto en esa celda no se
// deforma: es un ConfigError (kind + path) y el mapa no se carga.
class MapSceneAdapter {
public:
	// `scale`: píxeles de pantalla por píxel de arte (entero > 0).
	explicit MapSceneAdapter(int scale);

	// ConfigError(MissingStructure, "maps.<id>") si el mapa no existe.
	SceneDescription adapt(const worldmodel::World& world,
			       const std::string& mapId) const;

private:
	void addTilesets(const worldmodel::World& world,
			 SceneDescription& scene) const;

	// Piso y caminos como capas en grilla (una celda de arte por celda
	// lógica), de abajo hacia arriba.
	void addLayers(const worldmodel::Map& map, const std::string& mapPath,
		       SceneDescription& scene) const;

	// Una capa; si no tiene ningún tile no se agrega.
	void addLayer(const worldmodel::TileGrid& grid, const std::string& path,
		      const worldmodel::Map& map, SceneDescription& scene) const;

	void addObjects(const worldmodel::Map& map,
			const std::string& mapPath, SceneDescription& scene) const;

	// Escala de dibujo de una hoja para que su celda de arte ocupe una
	// celda lógica exacta. Lanza ConfigError(InvalidValue, `path`) si no
	// divide exacto.
	int drawScaleFor(const TileSetDef& tileset, int cellPixelSize,
			 const std::string& path) const;

	int scale_;
};

}
