#include "WorldSprites.h"

#include <QColor>
#include <QImage>
#include <QMargins>
#include <QRect>
#include <QtGlobal>
#include <map>

#include "NineSlice.h"
#include "WorldSpriteRects.h"

using namespace WorldSpriteRects;

WorldSprites::WorldSprites():
        menuTiles_(load(kMenuTilesPath)),
        iconSheet_(load(kWorldIconsPath)),
        frameBlock_(menuTiles_.isNull() ?
                            QPixmap() :
                            menuTiles_.copy(QRect(kFrameNormalSourceX, kFrameNormalSourceY,
                                                  kFrameVisibleSize, kFrameVisibleSize))),
        selectedBorder_(
                menuTiles_.isNull() ?
                        QPixmap() :
                        NineSlice(
                                menuTiles_.copy(QRect(kFrameSelectedSourceX, kFrameSelectedSourceY,
                                                      kFrameSourceSize, kFrameSourceSize)),
                                QMargins(kFrameSlice, kFrameSlice, kFrameSlice, kFrameSlice))
                                .build(kIconWidth, kIconHeight, kFrameScale)) {
    buildIcons();
}

QPixmap WorldSprites::panelFrame(int width, int height) const {
    if (frameBlock_.isNull()) {
        return QPixmap();
    }
    return NineSlice(frameBlock_, QMargins(kFrameSlice, kFrameSlice, kFrameSlice, kFrameSlice))
            .build(width, height, kFrameScale);
}

const QPixmap& WorldSprites::selectedBorder() const { return selectedBorder_; }

const QPixmap& WorldSprites::iconFor(std::uint32_t worldId) const {
    return iconFrom(icons_, worldId);
}

const QPixmap& WorldSprites::hoverIconFor(std::uint32_t worldId) const {
    return iconFrom(hoverIcons_, worldId);
}

QPixmap WorldSprites::load(const char* path) const {
    QPixmap sprite(path);
    if (sprite.isNull()) {
        qWarning("WorldSprites: %s no carga (alias mal escrito en el .qrc o archivo ausente).",
                 path);
    }
    return sprite;
}

const QPixmap& WorldSprites::iconFrom(const std::map<std::uint32_t, QPixmap>& icons,
                                      std::uint32_t worldId) const {
    const auto icon = icons.find(worldId);
    if (icon != icons.end()) {
        return icon->second;
    }
    return icons.at(kDefaultWorldId);
}

QPixmap WorldSprites::brightenPercent(const QPixmap& sprite) const {
    QImage image = sprite.toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const QColor color = image.pixelColor(x, y);
            if (color.alpha() == 0) {
                continue;
            }
            image.setPixelColor(x, y, color.lighter(kHoverBrightPercent));
        }
    }
    return QPixmap::fromImage(image);
}

void WorldSprites::buildIcons() {
    for (int index = 0; index < kIconCount; ++index) {
        const QPixmap cell = iconSheet_.copy(
                QRect(index * kIconNativeWidth, 0, kIconNativeWidth, kIconNativeHeight));
        const QPixmap icon =
                cell.scaled(kIconWidth, kIconHeight, Qt::IgnoreAspectRatio, Qt::FastTransformation);
        const auto id = static_cast<std::uint32_t>(kFirstWorldId + index);
        icons_.emplace(id, icon);
        hoverIcons_.emplace(id, brightenPercent(icon));
    }
}
