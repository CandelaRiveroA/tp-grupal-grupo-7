#include "world/WorldMapLayout.h"

#include <algorithm>

namespace world {

WorldMapLayout::WorldMapLayout(const int screenW, const int screenH,
                               const WorldMapConfig& config,
                               const PixelSize& mapTextureSize)
    : bgInset_(config.bgInset)
    , mapBorder_(config.mapBorder)
    , pinSize_(config.pinSize)
    , pinBorder_(config.pinBorder) {
    const int panelW = std::max(
            config.minWidth, static_cast<int>(static_cast<double>(screenW) * config.widthFraction));
    const int panelH = std::max(
            config.minHeight,
            static_cast<int>(static_cast<double>(screenH) * config.heightFraction));
    panel_ = PixelBox{
            (screenW - panelW) / 2,
            (screenH - panelH) / 2,
            panelW,
            panelH,
    };

    titleBox_ = PixelBox{
            panel_.x + config.framePad,
            panel_.y + config.titleTopPad,
            panel_.width - 2 * config.framePad,
            config.titleHeight,
    };

    const int mapTop = titleBox_.y + titleBox_.height + config.mapTopGap;
    const int mapBottom = panel_.y + panel_.height - config.framePad;
    const int mapAreaH = std::max(1, mapBottom - mapTop);
    const int arrowY = mapTop + (mapAreaH - config.arrowHeight) / 2;

    leftArrowRect_ = PixelBox{
            panel_.x + config.arrowMargin,
            arrowY,
            config.arrowWidth,
            config.arrowHeight,
    };
    rightArrowRect_ = PixelBox{
            panel_.x + panel_.width - config.arrowMargin - config.arrowWidth,
            arrowY,
            config.arrowWidth,
            config.arrowHeight,
    };

    const int innerLeft = leftArrowRect_.x + leftArrowRect_.width + config.arrowGap;
    const int innerRight = rightArrowRect_.x - config.arrowGap;
    const PixelBox mapArea{
            innerLeft,
            mapTop,
            std::max(1, innerRight - innerLeft),
            mapAreaH,
    };
    mapRect_ = fitMapRect(mapArea, mapTextureSize);
}

const PixelBox& WorldMapLayout::panel() const { return panel_; }

PixelBox WorldMapLayout::backgroundBox() const {
    return PixelBox{
            panel_.x + bgInset_,
            panel_.y + bgInset_,
            std::max(0, panel_.width - 2 * bgInset_),
            std::max(0, panel_.height - 2 * bgInset_),
    };
}

const PixelBox& WorldMapLayout::titleBox() const { return titleBox_; }

const PixelBox& WorldMapLayout::mapRect() const { return mapRect_; }

PixelBox WorldMapLayout::mapBorderBox() const {
    return PixelBox{
            mapRect_.x - mapBorder_,
            mapRect_.y - mapBorder_,
            mapRect_.width + 2 * mapBorder_,
            mapRect_.height + 2 * mapBorder_,
    };
}

const PixelBox& WorldMapLayout::leftArrowRect() const { return leftArrowRect_; }

const PixelBox& WorldMapLayout::rightArrowRect() const { return rightArrowRect_; }

PixelBox WorldMapLayout::pinOuterBox(const float normX, const float normY) const {
    const int centerX =
            mapRect_.x + static_cast<int>(normX * static_cast<float>(mapRect_.width));
    const int centerY =
            mapRect_.y + static_cast<int>(normY * static_cast<float>(mapRect_.height));
    return PixelBox{
            centerX - pinSize_ / 2,
            centerY - pinSize_ / 2,
            pinSize_,
            pinSize_,
    };
}

PixelBox WorldMapLayout::pinInnerBox(const PixelBox& outer) const {
    const int innerSize = std::max(1, outer.width - 2 * pinBorder_);
    return PixelBox{
            outer.x + pinBorder_,
            outer.y + pinBorder_,
            innerSize,
            std::max(1, outer.height - 2 * pinBorder_),
    };
}

bool WorldMapLayout::hitsSwitchArrow(const int x, const int y) const {
    return contains(leftArrowRect_, x, y) || contains(rightArrowRect_, x, y);
}

PixelBox WorldMapLayout::fitMapRect(const PixelBox& area,
                                    const PixelSize& textureSize) const {
    if (textureSize.width <= 0 || textureSize.height <= 0 ||
        area.width <= 0 || area.height <= 0) {
        return area;
    }

    const double scaleX =
            static_cast<double>(area.width) / static_cast<double>(textureSize.width);
    const double scaleY =
            static_cast<double>(area.height) / static_cast<double>(textureSize.height);
    const double scale = std::min(scaleX, scaleY);
    const int fitW = std::max(1, static_cast<int>(textureSize.width * scale));
    const int fitH = std::max(1, static_cast<int>(textureSize.height * scale));
    return PixelBox{
            area.x + (area.width - fitW) / 2,
            area.y + (area.height - fitH) / 2,
            fitW,
            fitH,
    };
}

bool WorldMapLayout::contains(const PixelBox& box, const int x, const int y) const {
    return x >= box.x && x < box.x + box.width && y >= box.y && y < box.y + box.height;
}

}  // namespace world
