#pragma once

#include <cstdint>

#include "world/Grid.h"

namespace world {

// Constantes de geometría del mundo.
constexpr int kOriginColumn = 0;
constexpr int kOriginRow = 0;
constexpr int kDefaultChunkSizePixels = 32;
constexpr GridSpan kUnitCellSpan{1, 1};

// Escala de dibujo por defecto de los mapas del editor (px de pantalla
// por px de arte) cuando el overlay no declara `[world] scale`.
constexpr int kDefaultWorldScale = 2;

// Bits de la máscara de vecinos del auto-tiling (world::CellPatch):
// qué vecinos cardinales de una celda pertenecen al mismo parche. El
// nombre de cada pieza del TOML se traduce a una combinación de estos
// bits (world/GroundTomlReader).
constexpr std::uint8_t kNeighborNorth = 1;
constexpr std::uint8_t kNeighborEast = 2;
constexpr std::uint8_t kNeighborSouth = 4;
constexpr std::uint8_t kNeighborWest = 8;

// Cara de fuente por defecto (la de [font] file en ui/text.toml). Las demás
// se declaran como [[font.face]] con nombre propio.
constexpr const char* kDefaultFontFace = "default";

// Nombres de los botones del popup ([prompt.<nombre>] en ui/text.toml).
constexpr const char* kPromptCancelButton = "cancel";
constexpr const char* kPromptOkButton = "ok";

// Rango del volumen que elige el jugador (porcentaje entero).
constexpr int kMinVolumePercent = 0;
constexpr int kMaxVolumePercent = 100;

// Tope de filas que se prueba al buscar cuántas necesita un popup
// (InventoryLayout::rowsFitting): corta el cálculo ante datos absurdos.
constexpr int kMaxPanelRows = 32;

}
