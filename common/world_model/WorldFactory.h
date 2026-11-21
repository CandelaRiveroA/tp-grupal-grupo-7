#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

#include "world_model/Constants.h"
#include "world_model/Geometry.h"
#include "world_model/Map.h"
#include "world_model/MapRules.h"
#include "world_model/PrefabCatalog.h"
#include "world_model/Scenario.h"
#include "world_model/ScenarioLibrary.h"
#include "world_model/Tile.h"
#include "world_model/World.h"

namespace worldmodel {

struct ScenarioChoice {
	std::string farm;
	std::string town;
	std::string cabin;
	std::string market;
	std::string communityCenter;
	std::array<std::string, kDungeonLevels> dungeons;
};

class WorldFactory {
public:
	World makeDefault(const std::string& name,
			  const ScenarioLibrary& library,
			  const PrefabCatalog& catalog,
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
				const Scenario& scenario,
				const PrefabCatalog& catalog = {}) const;
	// Elimina un mapa extra existente
	void removeExtraMap(World& world, const std::string& mapId) const;

private:
	// Coloca el escenario en el slot obligatorio mapId y coloca sus
	void addMandatoryMap(World& world, const std::string& mapId,
			     const Scenario& scenario,
			     const PrefabCatalog& catalog) const;
	Map copyScenario(const Scenario& scenario, const std::string& id,
			 MapKind kind, int level) const;
	void mergeTilesets(World& world, const Scenario& scenario) const;
	void mergePrefabTileset(World& world, const PrefabCatalog& catalog,
				const std::string& tilesetId) const;
	TileRef defaultObjectTile(const World& world) const;
	void placeMapObjects(World& world, const std::string& mapId,
			     const PrefabCatalog& catalog) const;
	void placeBuilding(World& world, Map& map, ObjectRole role,
			   TilePos cell, int defaultWidth, int defaultHeight,
			   TilePos defaultDoorOffset,
			   const PrefabCatalog& catalog) const;
	void placeMapPoints(World& world, const std::string& mapId) const;
	void addObject(Map& map, ObjectRole role, const TileRef& tile,
		       TilePos cell, int width, int height, bool blocked,
		       std::optional<TilePos> door,
		       std::vector<bool> collisionGrid) const;
	void addMarker(Map& map, MarkerKind kind, TilePos cell) const;
	void addPortal(Map& map, PortalKind kind, TilePos cell) const;
	void wirePortals(World& world) const;

	MapRules rules_;
};

}
