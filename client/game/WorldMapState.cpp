#include "game/WorldMapState.h"

#include <algorithm>
#include <cstddef>
#include <unordered_set>
#include <utility>

#include "common/world_model/Portal.h"
#include "common/world_model/WorldVocabulary.h"
#include "world/SceneDescription.h"
#include "world/SceneSource.h"

namespace game {

void WorldMapState::configure(WorldMapAnchors anchors,
                              std::vector<std::string> exteriors) {
    anchors_ = std::move(anchors);
    exteriors_ = std::move(exteriors);
    if (!exteriors_.empty() && !isExterior(activeExterior_)) {
        activeExterior_ = exteriors_.front();
    }
}

void WorldMapState::configureFromSources(
        const std::vector<const world::SceneSource*>& sources,
        std::vector<std::string> exteriors) {
    exteriors_ = std::move(exteriors);
    if (!exteriors_.empty() && !isExterior(activeExterior_)) {
        activeExterior_ = exteriors_.front();
    }

    WorldMapAnchors anchors;
    const worldmodel::WorldVocabulary vocabulary;
    const std::string dungeonEntranceKind =
            vocabulary.toText(worldmodel::PortalKind::DungeonEntrance);
    const std::string interiorExitKind =
            vocabulary.toText(worldmodel::PortalKind::InteriorExit);

    std::unordered_set<std::string> seen;
    for (const world::SceneSource* source : sources) {
        if (source == nullptr) {
            continue;
        }
        for (const std::string& id : source->ids()) {
            if (!seen.insert(id).second || rules_.dungeonLevel(id).has_value()) {
                continue;
            }
            const world::SceneDescription scene = source->load(id);
            if (isExterior(id)) {
                anchors.exteriors.emplace(id, scene.grid);
                for (const world::ScenePortal& portal : scene.portals) {
                    if (portal.kind == dungeonEntranceKind) {
                        anchors.dungeonExterior = id;
                        anchors.dungeonEntrance = portal.cell;
                        break;
                    }
                }
                continue;
            }
            // Cada mapa interior declara su propio portal interior_exit con
            // la celda exacta de salida en su exterior (incluidas cabin_1..4).
            for (const world::ScenePortal& portal : scene.portals) {
                if (portal.kind == interiorExitKind) {
                    anchors.interiorAnchors.emplace(
                            id, WorldMapInteriorAnchor{portal.targetMap, portal.targetCell});
                    break;
                }
            }
        }
    }
    anchors_ = std::move(anchors);
}

void WorldMapState::toggle(const std::string& localMapId) {
    if (open_) {
        open_ = false;
        return;
    }
    open_ = true;
    activeExterior_ = resolveExteriorFor(localMapId);
}

void WorldMapState::close() {
    open_ = false;
}

bool WorldMapState::isOpen() const {
    return open_;
}

const std::string& WorldMapState::activeExterior() const {
    return activeExterior_;
}

void WorldMapState::switchExterior() {
    if (exteriors_.empty()) {
        return;
    }
    const auto it = std::find(exteriors_.begin(), exteriors_.end(), activeExterior_);
    if (it == exteriors_.end()) {
        activeExterior_ = exteriors_.front();
        return;
    }
    const std::size_t index = static_cast<std::size_t>(it - exteriors_.begin());
    activeExterior_ = exteriors_[(index + 1) % exteriors_.size()];
}

std::vector<WorldMapPin> WorldMapState::pinsForActive(
        const std::vector<ViewMapPlayer>& players) const {
    const auto extIt = anchors_.exteriors.find(activeExterior_);
    if (extIt == anchors_.exteriors.end() ||
        extIt->second.columns <= 0 || extIt->second.rows <= 0) {
        return {};
    }
    const float width = static_cast<float>(extIt->second.columns);
    const float height = static_cast<float>(extIt->second.rows);

    std::vector<WorldMapPin> pins;
    pins.reserve(players.size());
    for (const ViewMapPlayer& player : players) {
        const std::optional<ExteriorTarget> target = projectPlayer(player);
        if (!target.has_value() || target->exteriorMapId != activeExterior_) {
            continue;
        }
        const float rawX = (static_cast<float>(target->cellX) + 0.5f) / width;
        const float rawY = (static_cast<float>(target->cellY) + 0.5f) / height;
        pins.push_back(WorldMapPin{
                player.id,
                std::clamp(rawX, 0.0f, 1.0f),
                std::clamp(rawY, 0.0f, 1.0f),
                player.appearance,
                player.isLocal,
                player.displayName,
        });
    }
    return pins;
}

bool WorldMapState::isExterior(const std::string& mapId) const {
    return std::find(exteriors_.begin(), exteriors_.end(), mapId) != exteriors_.end();
}

std::string WorldMapState::resolveExteriorFor(const std::string& mapId) const {
    if (const std::optional<ExteriorTarget> target = projectPlayer(ViewMapPlayer{0, mapId});
        target.has_value() && isExterior(target->exteriorMapId)) {
        return target->exteriorMapId;
    }
    if (const worldmodel::MapRule* rule = rules_.find(mapId);
        rule != nullptr && rule->interiorExitTarget.has_value() &&
        isExterior(*rule->interiorExitTarget)) {
        return *rule->interiorExitTarget;
    }
    return exteriors_.empty() ? std::string() : exteriors_.front();
}

std::optional<WorldMapState::ExteriorTarget> WorldMapState::projectPlayer(
        const ViewMapPlayer& player) const {
    if (isExterior(player.mapId)) {
        return ExteriorTarget{player.mapId, player.cellX, player.cellY};
    }
    if (rules_.dungeonLevel(player.mapId).has_value()) {
        if (!anchors_.dungeonEntrance.has_value() || anchors_.dungeonExterior.empty()) {
            return std::nullopt;
        }
        return ExteriorTarget{anchors_.dungeonExterior, anchors_.dungeonEntrance->column,
                              anchors_.dungeonEntrance->row};
    }
    if (const auto it = anchors_.interiorAnchors.find(player.mapId);
        it != anchors_.interiorAnchors.end()) {
        return ExteriorTarget{
                it->second.exteriorMapId,
                it->second.doorCell.column,
                it->second.doorCell.row,
        };
    }
    return std::nullopt;
}

}  // namespace game
