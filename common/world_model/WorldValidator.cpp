#include "world_model/WorldValidator.h"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace worldmodel {

bool WorldValidator::markerAllowedIn(const std::string& mapId, MarkerKind kind) const {
    const MapRule* rule = rules_.find(mapId);
    if (rule == nullptr) {
        return true;  // los mapas extra admiten cualquier marcador
    }
    return std::find(rule->allowedMarkers.begin(), rule->allowedMarkers.end(), kind) !=
           rule->allowedMarkers.end();
}

bool WorldValidator::portalAllowedIn(const std::string& mapId, PortalKind kind) const {
    if (kind == PortalKind::Link) {
        return true;
    }
    const MapRule* rule = rules_.find(mapId);
    return rule != nullptr &&
           std::find(rule->requiredPortals.begin(), rule->requiredPortals.end(), kind) !=
                   rule->requiredPortals.end();
}

const MapObject* WorldValidator::findBuilding(const Map& map, const MapRule& interiorRule) const {
    if (!interiorRule.interiorObject.has_value()) {
        return nullptr;
    }
    const ObjectRole role = *interiorRule.interiorObject;
    const std::optional<int> cabinIdx = rules_.cabinIndex(interiorRule.id);
    int currentCabin = 0;
    for (const MapObject& object: map.objects) {
        if (object.role != role) {
            continue;
        }
        if (role != ObjectRole::Cabin) {
            return &object;
        }
        ++currentCabin;
        if (cabinIdx.has_value() && currentCabin == *cabinIdx) {
            return &object;
        }
    }
    return nullptr;
}

int WorldValidator::countRole(const Map& map, ObjectRole role) const {
    int count = 0;
    for (const MapObject& object: map.objects) {
        if (object.role == role) {
            ++count;
        }
    }
    return count;
}

int WorldValidator::countMarker(const Map& map, MarkerKind kind) const {
    int count = 0;
    for (const Marker& marker: map.markers) {
        if (marker.kind == kind) {
            ++count;
        }
    }
    return count;
}

int WorldValidator::countPortal(const Map& map, PortalKind kind) const {
    int count = 0;
    for (const Portal& portal: map.portals) {
        if (portal.kind == kind) {
            ++count;
        }
    }
    return count;
}

void WorldValidator::addIssue(std::vector<ValidationIssue>& issues, IssueCode code,
                              const std::string& mapId, TilePos cell) const {
    issues.push_back(ValidationIssue{code, mapId, cell});
}

void WorldValidator::requireUniqueRole(const Map& map, ObjectRole role,
                                       std::vector<ValidationIssue>& issues) const {
    const int count = countRole(map, role);
    if (count == 0) {
        addIssue(issues, IssueCode::MissingObject, map.id);
        return;
    }
    if (count > 1) {
        addIssue(issues, IssueCode::DuplicateObject, map.id);
    }
}

void WorldValidator::requireMarker(const Map& map, MarkerKind kind,
                                   std::vector<ValidationIssue>& issues) const {
    if (countMarker(map, kind) == 0) {
        addIssue(issues, IssueCode::MissingMarker, map.id);
    }
}

void WorldValidator::requireUniqueMarker(const Map& map, MarkerKind kind,
                                         std::vector<ValidationIssue>& issues) const {
    const int count = countMarker(map, kind);
    if (count == 0) {
        addIssue(issues, IssueCode::MissingMarker, map.id);
    } else if (count > 1) {
        addIssue(issues, IssueCode::DuplicateMarker, map.id);
    }
}


std::vector<ValidationIssue> WorldValidator::validate(const World& world) const {
    std::vector<ValidationIssue> issues;
    checkWorld(world, issues);
    checkRequiredMaps(world, issues);
    for (const auto& entry: world.maps) {
        checkMap(world, entry.second, issues);
    }
    return issues;
}

void WorldValidator::checkWorld(const World& world, std::vector<ValidationIssue>& issues) const {
    if (world.formatVersion != 1) {
        addIssue(issues, IssueCode::UnsupportedVersion, "");
    }
    if (world.name.empty()) {
        addIssue(issues, IssueCode::EmptyName, "");
    } else if (!rules_.isValidWorldName(world.name)) {
        addIssue(issues, IssueCode::InvalidName, "");
    }
}

void WorldValidator::checkRequiredMaps(const World& world,
                                       std::vector<ValidationIssue>& issues) const {
    const auto farmIt = world.maps.find("farm");
    const int rawCabins =
            farmIt != world.maps.end() ? countRole(farmIt->second, ObjectRole::Cabin) : 0;
    const int expectedCabins = std::clamp(rawCabins, kMinPlayers, kMaxPlayers);

    for (const auto& [key, map]: world.maps) {
        const std::optional<int> cabinIdx = rules_.cabinIndex(key);
        if (!rules_.isValidMapId(key) || key != map.id ||
            (cabinIdx.has_value() && *cabinIdx > expectedCabins)) {
            addIssue(issues, IssueCode::InvalidMapId, key);
        }
        const MapRule* rule = rules_.find(key);
        if (rule == nullptr) {
            if (map.kind != MapKind::Dungeon && map.level != 0) {
                addIssue(issues, IssueCode::WrongDungeonLevel, key);
            }
            continue;
        }
        if (map.kind != rule->kind) {
            addIssue(issues, IssueCode::WrongMapKind, key);
        }
        const int expectedLevel = rules_.dungeonLevel(key).value_or(0);
        if (map.level != expectedLevel) {
            addIssue(issues, IssueCode::WrongDungeonLevel, key);
        }
    }

    for (const std::string& id: rules_.requiredMapIds(expectedCabins)) {
        if (world.maps.find(id) == world.maps.end()) {
            addIssue(issues, IssueCode::MissingRequiredMap, id);
        }
    }
}

void WorldValidator::checkMap(const World& world, const Map& map,
                              std::vector<ValidationIssue>& issues) const {
    if (map.width <= 0 || map.height <= 0 || map.tileSize <= 0) {
        addIssue(issues, IssueCode::InvalidDimensions, map.id);
        return;
    }

    const std::size_t cells =
            static_cast<std::size_t>(map.width) * static_cast<std::size_t>(map.height);
    const bool collisionSizesMatch =
            map.baseCollision.size() == cells && map.collision.size() == cells;
    if (map.baseCollision.size() != cells) {
        addIssue(issues, IssueCode::GridSizeMismatch, map.id);
    }
    if (map.collision.size() != cells) {
        addIssue(issues, IssueCode::GridSizeMismatch, map.id);
    }
    if (map.ground.size() != cells) {
        addIssue(issues, IssueCode::GridSizeMismatch, map.id);
    }
    if (map.paths.size() != cells) {
        addIssue(issues, IssueCode::GridSizeMismatch, map.id);
    }
    if (collisionSizesMatch && map.collision != map.composeCollision()) {
        addIssue(issues, IssueCode::CollisionOutOfSync, map.id);
    }

    const auto checkGridTiles = [&](const TileGrid& grid) {
        for (std::size_t i = 0; i < grid.size(); ++i) {
            const std::optional<TileRef>& tile = grid[i];
            if (!tile.has_value() || world.tilesets.find(tile->tileset) != world.tilesets.end()) {
                continue;
            }
            const TilePos cell{static_cast<int>(i % static_cast<std::size_t>(map.width)),
                               static_cast<int>(i / static_cast<std::size_t>(map.width))};
            addIssue(issues, IssueCode::UnknownTileset, map.id, cell);
        }
    };
    checkGridTiles(map.ground);
    checkGridTiles(map.paths);

    checkObjects(world, map, issues);
    checkMarkers(map, issues);
    checkPortals(world, map, issues);
}

void WorldValidator::checkObjects(const World& world, const Map& map,
                                  std::vector<ValidationIssue>& issues) const {
    const MapRule* rule = rules_.find(map.id);
    if (rule != nullptr) {
        for (const ObjectRole role: rule->requiredObjects) {
            requireUniqueRole(map, role, issues);
        }
    }
    if (map.id == "farm") {
        const int cabins = countRole(map, ObjectRole::Cabin);
        if (cabins == 0) {
            addIssue(issues, IssueCode::MissingObject, map.id);
        } else if (cabins > kMaxPlayers) {
            addIssue(issues, IssueCode::DuplicateObject, map.id);
        }
    }

    for (const MapObject& object: map.objects) {
        const bool needsDoor = object.role == ObjectRole::Cabin ||
                               object.role == ObjectRole::Market ||
                               object.role == ObjectRole::CommunityCenter;
        if (needsDoor) {
            if (!object.door.has_value()) {
                addIssue(issues, IssueCode::ObjectDoorMissing, map.id, object.cell);
            } else if (!map.contains(*object.door)) {
                addIssue(issues, IssueCode::ObjectDoorOutOfBounds, map.id, object.cell);
            } else if (!map.isWalkable(*object.door)) {
                addIssue(issues, IssueCode::ObjectDoorNotWalkable, map.id, object.cell);
            }
        }

        const bool inside = object.cell.column >= 0 && object.cell.row >= 0 &&
                            object.cell.column + object.width <= map.width &&
                            object.cell.row + object.height <= map.height;
        if (!inside) {
            addIssue(issues, IssueCode::ObjectOutOfBounds, map.id, object.cell);
        }

        if (world.tilesets.find(object.tile.tileset) == world.tilesets.end()) {
            addIssue(issues, IssueCode::UnknownTileset, map.id, object.cell);
        }
    }
}

void WorldValidator::checkMarkers(const Map& map, std::vector<ValidationIssue>& issues) const {
    for (const Marker& marker: map.markers) {
        if (!markerAllowedIn(map.id, marker.kind)) {
            addIssue(issues, IssueCode::UnexpectedMarker, map.id, marker.cell);
        }
        if (!map.contains(marker.cell)) {
            addIssue(issues, IssueCode::MarkerOutOfBounds, map.id, marker.cell);
        } else if (!map.isWalkable(marker.cell)) {
            addIssue(issues, IssueCode::MarkerNotWalkable, map.id, marker.cell);
        }
    }

    const MapRule* rule = rules_.find(map.id);
    if (rule != nullptr) {
        for (const MarkerRule& required: rule->requiredMarkers) {
            if (required.unique) {
                requireUniqueMarker(map, required.kind, issues);
            } else {
                requireMarker(map, required.kind, issues);
            }
        }
    }
}

void WorldValidator::checkPortals(const World& world, const Map& map,
                                  std::vector<ValidationIssue>& issues) const {
    const MapRule* rule = rules_.find(map.id);
    for (const Portal& portal: map.portals) {
        const bool allowed = portalAllowedIn(map.id, portal.kind);
        if (!allowed) {
            addIssue(issues, IssueCode::UnexpectedPortal, map.id, portal.cell);
        }

        if (!map.contains(portal.cell)) {
            addIssue(issues, IssueCode::PortalOutOfBounds, map.id, portal.cell);
        } else if (!map.isWalkable(portal.cell)) {
            addIssue(issues, IssueCode::PortalNotWalkable, map.id, portal.cell);
        }

        const auto target = world.maps.find(portal.targetMap);
        if (target == world.maps.end()) {
            addIssue(issues, IssueCode::PortalTargetMissing, map.id, portal.cell);
            continue;
        }

        const std::optional<std::string> expected =
                allowed && portal.kind != PortalKind::Link ?
                        rules_.expectedPortalTarget(map.id, portal.kind) :
                        std::nullopt;
        if (expected.has_value() && portal.targetMap != *expected) {
            addIssue(issues, IssueCode::WrongPortalTarget, map.id, portal.cell);
        }
        if (portal.kind == PortalKind::InteriorExit && expected.has_value() &&
            portal.targetMap == *expected) {
            const MapObject* building =
                    rule != nullptr ? findBuilding(target->second, *rule) : nullptr;
            if (building != nullptr && building->door.has_value() &&
                !(*building->door == portal.targetCell)) {
                addIssue(issues, IssueCode::WrongPortalTarget, map.id, portal.cell);
            }
        }

        if (!target->second.contains(portal.targetCell)) {
            addIssue(issues, IssueCode::PortalTargetOutOfBounds, map.id, portal.cell);
        } else if (!target->second.isWalkable(portal.targetCell)) {
            addIssue(issues, IssueCode::PortalTargetNotWalkable, map.id, portal.cell);
        }
    }

    if (rule != nullptr) {
        for (const PortalKind kind: rule->requiredPortals) {
            const int count = countPortal(map, kind);
            if (count == 0) {
                addIssue(issues, IssueCode::MissingPortal, map.id);
            } else if (count > 1) {
                addIssue(issues, IssueCode::DuplicatePortal, map.id);
            }
        }
    }
}

}  // namespace worldmodel
