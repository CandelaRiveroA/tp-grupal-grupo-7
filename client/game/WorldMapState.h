#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/model/draft/CharacterAppearance.h"
#include "common/world_model/MapRules.h"
#include "game/LocalWorldView.h"
#include "world/Grid.h"

namespace world {
class SceneSource;
}

namespace game {

// Destino de proyección de un mapa interior hacia su exterior y puerta.
struct WorldMapInteriorAnchor {
    std::string exteriorMapId;
    world::GridCell doorCell{0, 0};
};

// Puntos de referencia del mundo necesarios para proyectar jugadores sobre
// los minimapas exteriores configurados.
struct WorldMapAnchors {
    std::unordered_map<std::string, world::GridSpan> exteriors;
    std::string dungeonExterior;
    std::optional<world::GridCell> dungeonEntrance;
    std::unordered_map<std::string, WorldMapInteriorAnchor> interiorAnchors;
};

// Marcador ya proyectado a coordenadas normalizadas [0..1] del exterior visible.
struct WorldMapPin {
    int playerId = 0;
    float normX = 0.0f;
    float normY = 0.0f;
    CharacterAppearance appearance{};
    bool isLocal = false;
    std::string displayName{};
};

// Estado puro (sin SDL) del mapa del mundo desplegable con la tecla M:
// apertura/cierre, exterior activo y proyección de jugadores.
class WorldMapState {
public:
    void configure(WorldMapAnchors anchors, std::vector<std::string> exteriors);
    void configureFromSources(const std::vector<const world::SceneSource*>& sources,
                              std::vector<std::string> exteriors);

    void toggle(const std::string& localMapId);
    void close();
    bool isOpen() const;

    const std::string& activeExterior() const;
    void switchExterior();

    std::vector<WorldMapPin> pinsForActive(const std::vector<ViewMapPlayer>& players) const;

private:
    struct ExteriorTarget {
        std::string exteriorMapId;
        int cellX = 0;
        int cellY = 0;
    };

    bool isExterior(const std::string& mapId) const;
    std::string resolveExteriorFor(const std::string& mapId) const;
    std::optional<ExteriorTarget> projectPlayer(const ViewMapPlayer& player) const;

    worldmodel::MapRules rules_;
    std::vector<std::string> exteriors_;
    WorldMapAnchors anchors_;
    bool open_ = false;
    std::string activeExterior_;
};

}  // namespace game
