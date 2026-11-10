#pragma once

#include <optional>
#include <string>
#include <vector>

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
	bool isValidMapId(const std::string& id) const;
	bool isValidSlug(const std::string& slug) const;
	std::string dungeonId(int level) const;
	std::optional<int> dungeonLevel(const std::string& id) const;

private:
	bool isBareId(const std::string& text, bool allowUnderscore) const;

	std::vector<MapRule> rules_;
};

}
