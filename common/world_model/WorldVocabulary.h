#pragma once

#include <map>
#include <optional>
#include <string>

#include "world_model/Map.h"
#include "world_model/MapObject.h"
#include "world_model/Marker.h"
#include "world_model/Portal.h"

namespace worldmodel {

class WorldVocabulary {
public:
	WorldVocabulary();

	std::string toText(MapKind kind) const;
	std::string toText(ObjectRole role) const;
	std::string toText(MarkerKind kind) const;
	std::string toText(PortalKind kind) const;

	std::optional<MapKind> mapKindFrom(const std::string& text) const;
	std::optional<ObjectRole> objectRoleFrom(const std::string& text) const;
	std::optional<MarkerKind> markerKindFrom(const std::string& text) const;
	std::optional<PortalKind> portalKindFrom(const std::string& text) const;

private:
	std::map<MapKind, std::string> mapKindTexts_;
	std::map<ObjectRole, std::string> objectRoleTexts_;
	std::map<MarkerKind, std::string> markerKindTexts_;
	std::map<PortalKind, std::string> portalKindTexts_;

	std::map<std::string, MapKind> mapKinds_;
	std::map<std::string, ObjectRole> objectRoles_;
	std::map<std::string, MarkerKind> markerKinds_;
	std::map<std::string, PortalKind> portalKinds_;
};

}
