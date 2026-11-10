#pragma once

#include <optional>
#include <string>

#include "world_model/Map.h"
#include "world_model/MapObject.h"
#include "world_model/Marker.h"
#include "world_model/Portal.h"

namespace worldmodel {

class WorldVocabulary {
public:
	std::string toText(MapKind kind) const;
	std::string toText(ObjectRole role) const;
	std::string toText(MarkerKind kind) const;
	std::string toText(PortalKind kind) const;

	std::optional<MapKind> mapKindFrom(const std::string& text) const;
	std::optional<ObjectRole> objectRoleFrom(const std::string& text) const;
	std::optional<MarkerKind> markerKindFrom(const std::string& text) const;
	std::optional<PortalKind> portalKindFrom(const std::string& text) const;
};

}
