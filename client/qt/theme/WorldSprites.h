#pragma once

#include <QPixmap>
#include <cstdint>
#include <map>

class WorldSprites {
public:
    WorldSprites();

    QPixmap panelFrame(int width, int height) const;
    const QPixmap& selectedBorder() const;
    const QPixmap& iconFor(std::uint32_t worldId) const;
    const QPixmap& hoverIconFor(std::uint32_t worldId) const;

private:
    QPixmap load(const char* path) const;
    QPixmap brightenPercent(const QPixmap& sprite) const;
    void buildIcons();
    const QPixmap& iconFrom(const std::map<std::uint32_t, QPixmap>& icons,
                            std::uint32_t worldId) const;

    QPixmap menuTiles_;
    QPixmap iconSheet_;
    QPixmap frameBlock_;
    QPixmap selectedBorder_;
    std::map<std::uint32_t, QPixmap> icons_;
    std::map<std::uint32_t, QPixmap> hoverIcons_;
};
