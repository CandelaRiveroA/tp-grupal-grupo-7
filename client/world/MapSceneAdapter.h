#pragma once

#include <string>

#include "world/SceneDescription.h"
#include "world/WorldOverlay.h"
#include "world_model/World.h"

namespace world {

// Traduce un mapa del editor (common/world_model) al modelo de escena
// del cliente. Es el único que conoce las dos formas; el detalle de cada
// parte vive en su propio adaptador (MapLayerAdapter, MapObjectAdapter,
// MapAnchorAdapter) y este solo los orquesta.
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

	const WorldOverlay& overlay_;
};

}
