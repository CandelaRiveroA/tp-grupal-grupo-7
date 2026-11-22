#pragma once

#include <string>
#include <vector>

#include "world/ConfigError.h"
#include "world/SceneSource.h"
#include "world/WorldOverlay.h"
#include "world_model/World.h"

namespace world {

// Los mapas del TOML del editor como fuente de escenas: el id es el id
// del mapa (`farm`, `town`, `market`, ...). El mundo se lee una vez y
// cada escena se arma al pedirla (MapSceneAdapter).
class EditorWorldSource : public SceneSource {
public:
	EditorWorldSource(worldmodel::World world, WorldOverlay overlay);

	// Entradas del overlay que no coinciden con nada del mundo (no son
	// fatales; las muestra quien arma la sesión).
	const std::vector<ConfigError>& findings() const;

	std::vector<std::string> ids() const override;

	bool has(const std::string& id) const override;

	SceneDescription load(const std::string& id) const override;

private:
	worldmodel::World world_;
	WorldOverlay overlay_;
	std::vector<ConfigError> findings_;
};

}
