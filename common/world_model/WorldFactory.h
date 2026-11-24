#pragma once

#include <array>
#include <string>

#include "world_model/Constants.h"
#include "world_model/Geometry.h"
#include "world_model/Map.h"
#include "world_model/MapRules.h"
#include "world_model/Scenario.h"
#include "world_model/ScenarioLibrary.h"
#include "world_model/World.h"

namespace worldmodel {

struct ScenarioChoice {
	std::string farm;
	std::string town;
	std::string cabin;
	std::string carpentry;
	std::string blacksmith;
	std::string generalStore;
	std::string ranch;
	std::string barn;
	std::array<std::string, kDungeonLevels> dungeons;
};

class WorldFactory {
public:
	WorldFactory() = default;

	World makeDefault(const std::string& name,
			  const ScenarioLibrary& library,
			  const ScenarioChoice& choice) const;
	// Elige el primer escenario de la biblioteca para cada tipo. Si falta lanza FormatError.
	ScenarioChoice firstOfEachKind(const ScenarioLibrary& library) const;
	// Copia un escenario como mapa extra; no coloca puntos ni portales.
	void addExtraMap(World& world, const Scenario& scenario,
			 const std::string& id) const;
	// Reemplaza el escenario de un mapa existente. En un slot obligatorio
	// exige el mismo kind, recoloca sus puntos por defecto y recablea los
	// portales; en un mapa extra copia el escenario sin puntos. Si falla,
	// world queda intacto y se lanza FormatError.
	void replaceMapScenario(World& world, const std::string& mapId,
				const Scenario& scenario) const;
	// Elimina un mapa extra existente
	void removeExtraMap(World& world, const std::string& mapId) const;
	void addCabinMap(World& world, int index,
			 const Scenario& scenario) const;
	// Recablea los portales InteriorExit segun las puertas unicas de cada mapa anfitrion.
	void wirePortals(World& world) const;

private:
	// Coloca el escenario en el slot obligatorio mapId y coloca sus puntos.
	void addMandatoryMap(World& world, const std::string& mapId,
			     const Scenario& scenario) const;
	Map copyScenario(const Scenario& scenario, const std::string& id,
			 MapKind kind, int level) const;
	void mergeTilesets(World& world, const Scenario& scenario) const;
	void placeMapPoints(World& world, const std::string& mapId) const;
	TilePos findWalkableNear(const Map& map, TilePos preferred) const;
	void addMarker(Map& map, MarkerKind kind, TilePos cell) const;
	void addPortal(Map& map, PortalKind kind, TilePos cell) const;

	MapRules rules_;
};

}
