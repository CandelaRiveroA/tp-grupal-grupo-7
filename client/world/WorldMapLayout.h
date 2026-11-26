#pragma once

#include "world/PixelBox.h"
#include "world/WorldMapConfig.h"

namespace world {

// Geometría pura  del overlay del mapa del mundo
class WorldMapLayout {
public:
    WorldMapLayout(int screenW, int screenH, const WorldMapConfig& config,
                   const PixelSize& mapTextureSize);

    const PixelBox& panel() const;
    PixelBox backgroundBox() const;
    const PixelBox& titleBox() const;
    const PixelBox& mapRect() const;
    PixelBox mapBorderBox() const;
    const PixelBox& leftArrowRect() const;
    const PixelBox& rightArrowRect() const;
    PixelBox pinOuterBox(float normX, float normY) const;
    PixelBox pinInnerBox(const PixelBox& outer) const;

    bool hitsSwitchArrow(int x, int y) const;

private:
    PixelBox fitMapRect(const PixelBox& area, const PixelSize& textureSize) const;
    bool contains(const PixelBox& box, int x, int y) const;

    PixelBox panel_{};
    PixelBox titleBox_{};
    PixelBox mapRect_{};
    PixelBox leftArrowRect_{};
    PixelBox rightArrowRect_{};
    int bgInset_ = 0;
    int mapBorder_ = 0;
    int pinSize_ = 0;
    int pinBorder_ = 0;
};

}  // namespace world
