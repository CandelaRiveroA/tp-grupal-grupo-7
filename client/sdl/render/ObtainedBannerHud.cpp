#include "sdl/render/ObtainedBannerHud.h"

#include <algorithm>

#include "core/Constants.h"
#include "sdl/context/SdlContext.h"
#include "world/ObtainedBannerLayout.h"

namespace sdl {

namespace {
// Separación del panel respecto del borde izquierdo de la ventana (px).
constexpr int kMarginX = 12;
}  // namespace

ObtainedBannerHud::ObtainedBannerHud(SdlContext& context,
                                     const world::ObtainedBannerCatalog& catalog,
                                     int inventoryBlockPx):
        context_(context),
        catalog_(catalog),
        frameSheet_(context, core::assetPath(catalog.frame.sheetImage), catalog.frame.cellWidth,
                    catalog.frame.cellHeight),
        iconSheets_(context),
        inventoryBlockPx_(inventoryBlockPx > 0 ? inventoryBlockPx : 16) {}

int ObtainedBannerHud::panelSide() const {
    const int side = static_cast<int>(inventoryBlockPx_ * catalog_.sizeFactor);
    return side > 0 ? side : inventoryBlockPx_;
}

int ObtainedBannerHud::borderThickness() const {
    // Grosor del marco proporcional al panel. panelSide/6 deja esquinas
    // cuadradas y un marco fino, para que el icono respire y quede
    // centrado sin pisar el borde (importante en paneles chicos).
    const int t = panelSide() / 6;
    return std::max(t, 1);
}

void ObtainedBannerHud::drawCell(const world::SheetCell& cell, int dx, int dy, int dw, int dh) {
    if (dw <= 0 || dh <= 0) {
        return;
    }
    frameSheet_.draw(cell.col, cell.row, SDL2pp::Rect(dx, dy, dw, dh));
}

void ObtainedBannerHud::drawPanel(int x, int y, int side) {
    const world::BannerFrame& f = catalog_.frame;
    const int t = std::min(borderThickness(), side / 2);
    const int innerW = side - 2 * t;
    const int innerH = side - 2 * t;
    const int rightX = x + side - t;
    const int bottomY = y + side - t;

    // Relleno (fondo) primero, para que los bordes queden encima.
    drawCell(f.fill, x + t, y + t, innerW, innerH);

    drawCell(f.top, x + t, y, innerW, t);
    drawCell(f.bottom, x + t, bottomY, innerW, t);
    drawCell(f.left, x, y + t, t, innerH);
    drawCell(f.right, rightX, y + t, t, innerH);

    drawCell(f.topLeft, x, y, t, t);
    drawCell(f.topRight, rightX, y, t, t);
    drawCell(f.bottomLeft, x, bottomY, t, t);
    drawCell(f.bottomRight, rightX, bottomY, t, t);
}

void ObtainedBannerHud::draw(const std::vector<game::ActiveBanner>& banners) {
    const int side = panelSide();
    const world::ObtainedBannerLayout layout(context_.width(), context_.height(), side, kMarginX,
                                             catalog_.stackGap, catalog_.iconMargin,
                                             catalog_.iconOffsetX, catalog_.iconOffsetY);

    for (const game::ActiveBanner& banner: banners) {
        if (!catalog_.hasIcon(banner.kind)) {
            continue;
        }
        const world::BannerBox panel = layout.panelBox(banner.stackIndex);
        drawPanel(panel.x, panel.y, panel.width);

        const world::BannerBox icon = layout.iconBox(panel);
        const world::SpriteVisual& iconVisual = catalog_.iconFor(banner.kind);
        SpriteSheet& iconSheet =
                iconSheets_.sheet(iconVisual.sheetImage, core::assetPath(iconVisual.sheetImage),
                                  iconVisual.cellWidth, iconVisual.cellHeight);
        iconSheet.draw(iconVisual.col, iconVisual.row,
                       SDL2pp::Rect(icon.x, icon.y, icon.width, icon.height));
    }
}

}  // namespace sdl
