#pragma once

#include <cstdint>

#include "world/Grid.h"

namespace world {

// Constantes de geometría del mundo.
constexpr int kOriginColumn = 0;
constexpr int kOriginRow = 0;
constexpr int kDefaultChunkSizePixels = 32;
constexpr GridSpan kUnitCellSpan{1, 1};

// Bits de la máscara de vecinos del auto-tiling (world::CellPatch):
// qué vecinos cardinales de una celda pertenecen al mismo parche. El
// nombre de cada pieza del TOML se traduce a una combinación de estos
// bits (world/GroundTomlReader).
constexpr std::uint8_t kNeighborNorth = 1;
constexpr std::uint8_t kNeighborEast = 2;
constexpr std::uint8_t kNeighborSouth = 4;
constexpr std::uint8_t kNeighborWest = 8;

}
