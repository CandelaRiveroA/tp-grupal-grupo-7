#pragma once

#include "world_model/Tile.h"

class QPainter;
class TilesetCache;

namespace worldmodel {
struct Map;
}

// Dibuja las capas visibles de un mapa (suelo, caminos, objetos y frente) en orden Z.
class WorldPainter {
public:
    void paintWorldLayers(QPainter* painter, const worldmodel::Map& map,
                          const TilesetCache& tilesets, int minCol, int maxCol, int minRow,
                          int maxRow) const;

private:
    void drawGridLayer(QPainter* painter, const worldmodel::Map& map, const TilesetCache& tilesets,
                       const worldmodel::TileGrid& grid, int ts, int minCol, int maxCol, int minRow,
                       int maxRow) const;
    void drawObjects(QPainter* painter, const worldmodel::Map& map, const TilesetCache& tilesets,
                     int ts, int minCol, int maxCol, int minRow, int maxRow) const;
};
