#pragma once

#include <string>
#include <vector>

#include "world_model/MapRules.h"
#include "world_model/ValidationIssue.h"
#include "world_model/World.h"

namespace worldmodel {

using WorldName = std::string;

class WorldRepository {
public:
	explicit WorldRepository(std::string directoryPath);

	// Un archivo ilegible figura con su nombre de archivo, para que el
	// usuario lo vea y el error aparezca al intentar abrirlo.
	std::vector<WorldName> listWorlds() const;

	bool exists(const std::string& name) const;
	World load(const std::string& name) const;
	std::string filePathFor(const std::string& name) const;
	// Ruta del PNG de minimapa para un mapa del mundo (<nombre>_minimaps/<mapId>.png).
	std::string minimapPathFor(const std::string& name,
				   const std::string& mapId) const;
	// Crea el directorio de minimapas y elimina PNGs de mapas ya borrados.
	bool prepareMinimapsDir(const World& world) const;

	std::vector<ValidationIssue> saveIfValid(const World& world) const;

private:
	std::string directoryPath_;
	MapRules rules_;
};

}  // namespace worldmodel
