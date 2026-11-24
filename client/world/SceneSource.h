#pragma once

#include <string>
#include <vector>

#include "world/SceneDescription.h"

namespace world {

// De dónde sale una escena (SOLID: se programa contra esta interfaz,
// no contra un formato). Hoy las salas de scenes.toml; mañana también
// los mapas del TOML del editor. El id es el nombre de la sala o del
// mapa. Las implementaciones traducen sus errores a ConfigError.
class SceneSource {
public:
	virtual ~SceneSource() = default;

	// Ids de todas las escenas que conoce, en orden estable.
	virtual std::vector<std::string> ids() const = 0;

	// ¿La fuente conoce esta escena?
	virtual bool has(const std::string& id) const = 0;

	// Arma la escena del id (precondición: has(id)). Lanza ConfigError
	// si los datos no son válidos.
	virtual SceneDescription load(const std::string& id) const = 0;
};

}
