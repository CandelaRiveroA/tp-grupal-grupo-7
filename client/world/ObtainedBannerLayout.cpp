#include "world/ObtainedBannerLayout.h"

#include "world/AnchorLayout.h"
#include "world/PixelBox.h"

namespace world {

namespace {

// Ancla del panel dentro de la ventana. La pila va anclada a la izquierda
// y centrada en vertical: eso es un CenterLeft de AnchorLayout (la misma
// fórmula de anclaje que overlays e inventario, §9).
constexpr Anchor kPanelAnchor = Anchor::CenterLeft;

}  // namespace

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
    // El primer panel (stackIndex 0) se ancla con AnchorLayout; la pila
    // crece hacia abajo sumando panel+gap por nivel.
    const AnchorLayout anchors;
    const PixelBox anchored = anchors.box(kPanelAnchor, marginX_, 0, panelSide_,
                                          panelSide_, windowWidth_, windowHeight_);
    BannerBox box;
    box.width = panelSide_;
    box.height = panelSide_;
    box.x = anchored.x;
    box.y = anchored.y + stackIndex * (panelSide_ + stackGap_);
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
