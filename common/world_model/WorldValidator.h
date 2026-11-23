#pragma once

#include <optional>
#include <string>
#include <vector>

#include "world_model/MapRules.h"
#include "world_model/ValidationIssue.h"
#include "world_model/World.h"

namespace worldmodel {

class WorldValidator {
public:
    std::vector<ValidationIssue> validate(const World& world) const;

private:
    bool markerAllowedIn(const std::string& mapId, MarkerKind kind) const;
    bool portalAllowedIn(const std::string& mapId, PortalKind kind) const;
    const MapObject* findBuilding(const Map& map, const MapRule& interiorRule) const;
    int countRole(const Map& map, ObjectRole role) const;
    int countMarker(const Map& map, MarkerKind kind) const;
    int countPortal(const Map& map, PortalKind kind) const;
    void addIssue(std::vector<ValidationIssue>& issues, IssueCode code, const std::string& mapId,
                  TilePos cell = {-1, -1}, const std::string& detail = "") const;
    void requireUniqueRole(const Map& map, ObjectRole role,
                           std::vector<ValidationIssue>& issues) const;
    void requireMarker(const Map& map, MarkerKind kind, std::vector<ValidationIssue>& issues) const;
    void requireUniqueMarker(const Map& map, MarkerKind kind,
                             std::vector<ValidationIssue>& issues) const;

    void checkWorld(const World& world, std::vector<ValidationIssue>& issues) const;
    void checkRequiredMaps(const World& world, std::vector<ValidationIssue>& issues) const;
    void checkMap(const World& world, const Map& map, std::vector<ValidationIssue>& issues) const;
    void checkObjects(const World& world, const Map& map,
                      std::vector<ValidationIssue>& issues) const;
    void checkMarkers(const Map& map, std::vector<ValidationIssue>& issues) const;
    void checkPortals(const World& world, const Map& map,
                      std::vector<ValidationIssue>& issues) const;
    void checkPortalArea(const Map& areaMap, TilePos origin, int width, int height,
                         const std::string& reportMapId, TilePos reportCell,
                         IssueCode outOfBoundsCode, IssueCode notWalkableCode,
                         std::vector<ValidationIssue>& issues) const;

    MapRules rules_;
};

}  // namespace worldmodel
