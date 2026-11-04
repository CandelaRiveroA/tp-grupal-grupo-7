#pragma once

#include <vector>

#include "game/ObtainedBannerQueue.h"
#include "sdl/render/SheetCache.h"
#include "sdl/render/SpriteSheet.h"
#include "world/ObtainedBannerCatalog.h"

namespace sdl {
class SdlContext;
}

namespace sdl {

// Dibuja la pila de carteles de "mineral obtenido". Cada cartel es un panel
// 9-slice (celdas de MenuTiles, vía el catálogo) con el icono del mineral
// (de ore.png) centrado adentro. Imita a InventoryHud: el catálogo y la
// geometría son datos; acá solo se blitea.
class ObtainedBannerHud {
public:
    ObtainedBannerHud(SdlContext& context, const world::ObtainedBannerCatalog& catalog,
                      int inventoryBlockPx);

    void draw(const std::vector<game::ActiveBanner>& banners);

private:
    // Lado del panel en px (inventoryBlockPx * sizeFactor del catálogo).
    int panelSide() const;

    // Grosor del borde del 9-slice en px (una celda escalada al panel).
    int borderThickness() const;

    void drawPanel(int x, int y, int side);

    void drawCell(const world::SheetCell& cell, int dx, int dy, int dw, int dh);

    SdlContext& context_;
    world::ObtainedBannerCatalog catalog_;
    SpriteSheet frameSheet_;
    SheetCache iconSheets_;
    int inventoryBlockPx_;
};

}  // namespace sdl
