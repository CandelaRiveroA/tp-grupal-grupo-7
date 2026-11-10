#pragma once

#include <array>
#include <optional>
#include <string>

#include "world_model/Geometry.h"
#include "world_model/Map.h"
#include "world_model/MapRules.h"
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
	std::array<std::string, 10> dungeons;   // dungeons[i] → dungeon_{i+1}
};

class WorldFactory {
public:
	World makeDefault(const std::string& slug, const std::string& name,
			  const ScenarioLibrary& library,
			  const ScenarioChoice& choice) const;
	// Copia un escenario como mapa extra; no coloca puntos ni portales.
	void addExtraMap(World& world, const Scenario& scenario,
			 const std::string& id) const;

private:
	Map copyScenario(const Scenario& scenario, const std::string& id,
			 MapKind kind, int level) const;
	void mergeTilesets(World& world, const Scenario& scenario) const;
	void placeObjects(World& world) const;
	void placePoints(World& world) const;
	void addObject(Map& map, ObjectRole role, const TileRef& tile,
		       TilePos cell, int width, int height, bool blocked,
		       std::optional<TilePos> door) const;
	void addMarker(Map& map, MarkerKind kind, TilePos cell) const;
	void addPortal(Map& map, PortalKind kind, TilePos cell) const;
	void wirePortals(World& world) const;

	MapRules rules_;
};

}
