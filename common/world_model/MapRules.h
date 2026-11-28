#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "world_model/Constants.h"
#include "world_model/Map.h"
#include "world_model/MapObject.h"
#include "world_model/Marker.h"
#include "world_model/Portal.h"

namespace worldmodel {

struct MarkerRule {
	MarkerKind kind;
	bool unique;
};

struct MarkerSpec {
	int height = 1;
	bool requiresWalkable = true;
	bool erasable = false;
};

struct MapRule {
	std::string id;
	MapKind kind;
	std::vector<MarkerKind> allowedMarkers;
	std::vector<MarkerRule> requiredMarkers;
	std::vector<ObjectRole> requiredObjects;
	std::vector<PortalKind> requiredPortals;
	std::optional<ObjectRole> interiorObject;
	std::optional<std::string> interiorExitTarget;
};

class MapRules {
public:
	MapRules();

	const std::vector<MapRule>& all() const;
	const MapRule* find(const std::string& id) const;
	std::vector<std::string> requiredMapIds(int cabinCount) const;
	// Mapa anfitrion donde debe colocarse un objeto con `role`, si aplica.
	std::optional<std::string> requiredHostMap(ObjectRole role) const;
	// IDs de interiores unicos (excluye cabanas por jugador).
	std::vector<std::string> fixedInteriorIds() const;
	bool isInteriorHost(const std::string& mapId) const;
	bool isValidMapId(const std::string& id) const;
	bool isValidWorldName(const std::string& name) const;
	std::string cabinId(int k) const;
	std::optional<int> cabinIndex(const std::string& id) const;
	std::string dungeonId(int level) const;
	std::optional<int> dungeonLevel(const std::string& id) const;
	std::optional<std::string> expectedPortalTarget(
		const std::string& mapId, PortalKind kind) const;
	MarkerSpec markerSpec(MarkerKind kind) const;
	bool markerAllowedIn(const std::string& mapId, MarkerKind kind) const;
	bool canPlaceMarker(const Map& map, MarkerKind kind,
			    TilePos cell) const;
	bool isMarkerUnique(const std::string& mapId, MarkerKind kind) const;

private:
	std::vector<MapRule> rules_;
	std::map<MarkerKind, MarkerSpec> markerSpecs_;
};

}
