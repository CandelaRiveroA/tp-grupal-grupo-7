#include "WorldPainter.h"

#include <QPainter>
#include <cstddef>

#include "world_model/Map.h"
#include "world_model/MapObject.h"

#include "TilesetCache.h"

void WorldPainter::drawGridLayer(QPainter* painter, const worldmodel::Map& map,
                                 const TilesetCache& tilesets, const worldmodel::TileGrid& grid,
                                 int ts, int minCol, int maxCol, int minRow, int maxRow) const {
    for (int row = minRow; row <= maxRow; ++row) {
        for (int col = minCol; col <= maxCol; ++col) {
            const std::size_t idx = map.cellIndex({col, row});
            if (idx < grid.size() && grid[idx].has_value()) {
                painter->drawPixmap(col * ts, row * ts, tilesets.tilePixmap(*grid[idx]));
            }
        }
    }
}

void WorldPainter::drawObjects(QPainter* painter, const worldmodel::Map& map,
                               const TilesetCache& tilesets, int ts, int minCol, int maxCol,
                               int minRow, int maxRow, bool buildingsOnly) const {
    for (const auto& obj: map.objects) {
        const bool isBuilding = obj.role != worldmodel::ObjectRole::Decoration;
        if (isBuilding != buildingsOnly) {
            continue;
        }
        if (obj.cell.column + obj.width - 1 < minCol || obj.cell.column > maxCol ||
            obj.cell.row + obj.height - 1 < minRow || obj.cell.row > maxRow) {
            continue;
        }
        painter->drawPixmap(obj.cell.column * ts, obj.cell.row * ts,
                            tilesets.tilePixmap(obj.tile, obj.width, obj.height));
    }
}

void WorldPainter::paintWorldLayers(QPainter* painter, const worldmodel::Map& map,
                                    const TilesetCache& tilesets, int minCol, int maxCol,
                                    int minRow, int maxRow) const {
    const int ts = map.tileSize;
    drawGridLayer(painter, map, tilesets, map.ground, ts, minCol, maxCol, minRow, maxRow);
    drawGridLayer(painter, map, tilesets, map.paths, ts, minCol, maxCol, minRow, maxRow);
    drawObjects(painter, map, tilesets, ts, minCol, maxCol, minRow, maxRow, false);
    drawGridLayer(painter, map, tilesets, map.front, ts, minCol, maxCol, minRow, maxRow);
    drawObjects(painter, map, tilesets, ts, minCol, maxCol, minRow, maxRow, true);
}
