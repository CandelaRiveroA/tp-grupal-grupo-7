#pragma once

#include <string>

#include "world/SceneDescription.h"
#include "world/WorldOverlay.h"
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
	// La escala y lo que el cliente agrega al mundo (variantes de noche,
	// decoraciones iniciales) salen del overlay, PRESTADO: debe vivir más
	// que el adaptador.
	explicit MapSceneAdapter(const WorldOverlay& overlay);

	// ConfigError(MissingStructure, "maps.<id>") si el mapa no existe.
	SceneDescription adapt(const worldmodel::World& world,
			       const std::string& mapId) const;

private:
	void addTilesets(const worldmodel::World& world,
			 SceneDescription& scene) const;

	// Suma las hojas propias del cliente; una hoja con el nombre de una
	// del editor pero otra definición es un conflicto.
	void mergeOverlayTilesets(SceneDescription& scene) const;

	// Decoraciones iniciales del mapa (overlay), encima de los objetos.
	void addExtraElements(const std::string& mapId,
			      SceneDescription& scene) const;

	// La versión de noche de un elemento del editor: misma pieza y
	// posición, con el origen en otra celda de otra hoja.
	ElementPlacement nightElement(const ElementPlacement& day,
				      const NightVariant& night,
				      const std::string& path,
				      const SceneDescription& scene) const;

	// Piso y caminos como capas en grilla (una celda de arte por celda
	// lógica), de abajo hacia arriba.
	void addLayers(const worldmodel::Map& map, const std::string& mapPath,
		       SceneDescription& scene) const;

	// Una capa; si no tiene ningún tile no se agrega.
	void addLayer(const worldmodel::TileGrid& grid, const std::string& path,
		      const worldmodel::Map& map, SceneDescription& scene) const;

	// Markers y portales del mapa como datos de la escena, y la celda
	// por defecto de los NPC que el overlay ata a un marker.
	void addAnchors(const worldmodel::Map& map, SceneDescription& scene) const;

	// `enter` de una decoración: la celda donde queda acostado sale del
	// marker del mapa (la misma que usa el server).
	ElementEnter enterFor(const OverlayEnter& enter,
			      const ElementPlacement& placement,
			      const worldmodel::Map& map,
			      const std::string& mapPath,
			      const std::string& key) const;

	// El edificio del editor como estructura de la escena (con el
	// comportamiento que el overlay le da a su rol).
	void addStructure(const worldmodel::MapObject& object,
			  std::size_t elementIndex, SceneDescription& scene) const;

	void addObjects(const worldmodel::Map& map,
			const std::string& mapPath, SceneDescription& scene) const;

	// Escala de dibujo de una hoja para que su celda de arte ocupe una
	// celda lógica exacta. Lanza ConfigError(InvalidValue, `path`) si no
	// divide exacto.
	int drawScaleFor(const TileSetDef& tileset, int cellPixelSize,
			 const std::string& path) const;

	const WorldOverlay& overlay_;
};

}
