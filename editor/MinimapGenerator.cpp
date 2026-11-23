#include "MinimapGenerator.h"

#include <QPainter>
#include <QString>

#include "world_model/Map.h"
#include "world_model/World.h"
#include "world_model/WorldRepository.h"

#include "Constants.h"
#include "TilesetCache.h"
#include "WorldPainter.h"

QImage MinimapGenerator::renderMap(const worldmodel::Map& map, const TilesetCache& tilesets) const {
    if (map.width <= 0 || map.height <= 0 || map.tileSize <= 0) {
        return QImage();
    }
    const int px = editor::kMinimapPxPerCell;
    QImage image(map.width * px, map.height * px, QImage::Format_ARGB32_Premultiplied);
    if (image.isNull()) {
        return QImage();
    }
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    const double scale = static_cast<double>(px) / map.tileSize;
    painter.scale(scale, scale);
    WorldPainter().paintWorldLayers(&painter, map, tilesets, 0, map.width - 1, 0, map.height - 1);
    return image;
}

bool MinimapGenerator::generate(const worldmodel::World& world, const TilesetCache& tilesets,
                                const worldmodel::WorldRepository& worlds,
                                QStringList& failedMaps) const {
    if (!worlds.prepareMinimapsDir(world)) {
        return false;
    }
    for (const auto& [mapId, map]: world.maps) {
        const QImage image = renderMap(map, tilesets);
        const QString path = QString::fromStdString(worlds.minimapPathFor(world.name, mapId));
        if (image.isNull() || !image.save(path, "PNG")) {
            failedMaps << QString::fromStdString(mapId);
        }
    }
    return failedMaps.isEmpty();
}
