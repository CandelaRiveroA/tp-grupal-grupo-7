#pragma once

#include <QImage>
#include <QStringList>

class TilesetCache;

namespace worldmodel {
struct Map;
struct World;
class WorldRepository;
}  // namespace worldmodel

// Renderiza y guarda un PNG por cada mapa del mundo usando las rutas de WorldRepository.
class MinimapGenerator {
public:
    bool generate(const worldmodel::World& world, const TilesetCache& tilesets,
                  const worldmodel::WorldRepository& worlds, QStringList& failedMaps) const;

private:
    QImage renderMap(const worldmodel::Map& map, const TilesetCache& tilesets) const;
};
