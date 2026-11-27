#pragma once

#include <cstddef>
#include <string>

#include "world/SceneDescription.h"
#include "world/WorldOverlay.h"
#include "world_model/Map.h"

namespace world {

// Los objetos del editor (decoraciones y edificios) como elementos de la
// escena, en el orden del archivo (= el de dibujo). Aplica lo que el
// overlay agrega a cada uno: variante de noche, entrada (la cama) y el
// comportamiento del click de los edificios.
class MapObjectAdapter {
public:
	// El overlay es PRESTADO: debe vivir más que el adaptador.
	explicit MapObjectAdapter(const WorldOverlay& overlay);

	void add(const worldmodel::Map& map, const std::string& mapPath,
		 SceneDescription& scene) const;

private:
	void addObject(const worldmodel::Map& map, const std::string& mapPath,
		       std::size_t index, SceneDescription& scene) const;

	// El elemento (de día, de pie) que dibuja el objeto: el bloque
	// contiguo de la hoja que el editor recorta, en las celdas que ocupa.
	ElementPlacement placementFor(const worldmodel::MapObject& object,
				      const std::string& path,
				      const SceneDescription& scene) const;

	// La versión de noche de un elemento: misma pieza y posición, con el
	// origen en otra celda de otra hoja.
	ElementPlacement nightElement(const ElementPlacement& day,
				      const NightVariant& night,
				      const std::string& path,
				      const SceneDescription& scene) const;

	// `enter` de una decoración: la celda donde queda acostado sale del
	// marker del mapa (la misma que usa el server).
	ElementEnter enterFor(const OverlayEnter& enter,
			      const ElementPlacement& placement,
			      const worldmodel::Map& map,
			      const std::string& mapPath,
			      const std::string& key) const;

	// El edificio como estructura de la escena (con el comportamiento
	// que el overlay le da a su rol).
	void addStructure(const worldmodel::MapObject& object,
			  std::size_t elementIndex, SceneDescription& scene) const;

	const WorldOverlay& overlay_;
};

}
