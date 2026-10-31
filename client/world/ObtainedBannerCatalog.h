#pragma once

#include <map>
#include <string>

#include "world/ResourceCatalog.h"

namespace world {

// Celda (col,row) de una hoja de sprites.
struct BannerCell {
    int col = 0;
    int row = 0;
};

// Las 9 piezas del panel 9-slice: una celda representativa por pieza, que
// se estira al dibujar (igual patrón que el marco del inventario).
struct BannerFrame {
    std::string sheetImage;
    int cellWidth = 0;
    int cellHeight = 0;

    BannerCell topLeft;
    BannerCell topRight;
    BannerCell bottomLeft;
    BannerCell bottomRight;
    BannerCell top;
    BannerCell bottom;
    BannerCell left;
    BannerCell right;
    BannerCell fill;
};

// Icono del mineral obtenido: bloque 1x1 de su hoja (ore.png).
struct BannerIcon {
    std::string sheetImage;
    int cellWidth = 0;
    int cellHeight = 0;
    int col = 0;
    int row = 0;
};

// Descripción completa del cartel de "mineral obtenido", como DATOS. La
// fuente (mock o servidor) decide CUÁNDO aparece; este catálogo, CÓMO se
// ve. No depende de SDL.
struct ObtainedBannerCatalog {
    BannerFrame frame;

    std::map<ResourceKind, BannerIcon> icons;

    // Lado del panel como múltiplo del bloque de inventario.
    double sizeFactor = 2.0;
    int iconMargin = 8;
    // Ajuste fino del icono (px) para compensar que el sprite del mineral
    // no está centrado en su celda. Positivo = derecha / abajo.
    int iconOffsetX = 0;
    int iconOffsetY = 0;
    int durationTicks = 120;
    int maxVisible = 4;
    int stackGap = 4;

    bool hasIcon(ResourceKind kind) const;

    const BannerIcon& iconFor(ResourceKind kind) const;
};

}  // namespace world
