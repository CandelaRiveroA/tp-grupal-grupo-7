#include "world/ObtainedBannerLayout.h"

namespace world {

ObtainedBannerLayout::ObtainedBannerLayout(int windowWidth, int windowHeight, int panelSide,
                                           int marginX, int stackGap, int iconMargin,
                                           int iconOffsetX, int iconOffsetY):
        windowWidth_(windowWidth),
        windowHeight_(windowHeight),
        panelSide_(panelSide),
        marginX_(marginX),
        stackGap_(stackGap),
        iconMargin_(iconMargin),
        iconOffsetX_(iconOffsetX),
        iconOffsetY_(iconOffsetY) {}

BannerBox ObtainedBannerLayout::panelBox(int stackIndex) const {
    BannerBox box;
    box.width = panelSide_;
    box.height = panelSide_;
    box.x = marginX_;
    // El primer panel (stackIndex 0) queda con su borde superior tal que
    // la pila arranca centrada en vertical; crece hacia abajo.
    const int top = (windowHeight_ - panelSide_) / 2;
    box.y = top + stackIndex * (panelSide_ + stackGap_);
    return box;
}

BannerBox ObtainedBannerLayout::iconBox(const BannerBox& panel) const {
    BannerBox box;
    const int side = panel.width - 2 * iconMargin_;
    const int clampedSide = side > 0 ? side : panel.width;
    box.width = clampedSide;
    box.height = clampedSide;
    box.x = panel.x + (panel.width - clampedSide) / 2 + iconOffsetX_;
    box.y = panel.y + (panel.height - clampedSide) / 2 + iconOffsetY_;
    return box;
}

}  // namespace world
