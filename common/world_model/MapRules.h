#pragma once

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
	bool isValidMapId(const std::string& id) const;
	bool isValidWorldName(const std::string& name) const;
	std::string cabinId(int k) const;
	std::optional<int> cabinIndex(const std::string& id) const;
	std::string dungeonId(int level) const;
	std::optional<int> dungeonLevel(const std::string& id) const;
	std::optional<std::string> expectedPortalTarget(
		const std::string& mapId, PortalKind kind) const;

private:
	std::vector<MapRule> rules_;
};

}
