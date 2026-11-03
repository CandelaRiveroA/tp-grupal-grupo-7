#pragma once

namespace WorldSpriteRects {

// Marco: bloque nativo de 64x64 con tiles de 16x16
constexpr int kFrameSourceSize = 64;
// El bloque normal ocupa 60x60: la celda de 64 trae 4 px transparentes a la
// derecha y abajo.
constexpr int kFrameVisibleSize = 60;
constexpr int kFrameSlice = 16;
constexpr int kFrameScale = 1;

// Bloques en MenuTiles.png.
constexpr int kFrameNormalSourceX = 0;
constexpr int kFrameNormalSourceY = 256;
constexpr int kFrameSelectedSourceX = 0;
constexpr int kFrameSelectedSourceY = 896;

// Sheet de iconos: 6 celdas de 22x20 en una fila, escaladas a x2.
constexpr int kIconNativeWidth = 22;
constexpr int kIconNativeHeight = 20;
constexpr int kIconCount = 6;
constexpr int kIconScale = 2;
constexpr int kIconWidth = kIconNativeWidth * kIconScale;
constexpr int kIconHeight = kIconNativeHeight * kIconScale;

// Boton: el icono rodeado por un tile de marco a cada lado.
constexpr int kWorldButtonWidth = kIconWidth + 2 * kFrameSlice;
constexpr int kWorldButtonHeight = kIconHeight + 2 * kFrameSlice;
constexpr int kIconOffsetX = (kWorldButtonWidth - kIconWidth) / 2;
constexpr int kIconOffsetY = (kWorldButtonHeight - kIconHeight) / 2;

// Grilla de mundos: 6 filas x 2 columnas.
constexpr int kMaxRowsPerColumn = 6;
constexpr int kMaxColumns = 2;
constexpr int kMaxWorlds = kMaxRowsPerColumn * kMaxColumns;
constexpr int kRowGap = 6;
constexpr int kColumnGap = 10;
constexpr int kPanelGap = 20;

constexpr int kHoverBrightPercent = 112;
constexpr double kDisabledOpacity = 0.5;

// Id del mundo -> celda del sheet: el primer id es la celda 0, el siguiente la 1 y asi
constexpr int kFirstWorldId = 1;
constexpr int kDefaultWorldId = kFirstWorldId;

constexpr const char* kMenuTilesPath = ":/ui/menu_tiles.png";
constexpr const char* kWorldIconsPath = ":/ui/world/icons.png";

}
