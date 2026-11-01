#pragma once

#include "world/Grid.h"

namespace world {

// Constantes de geometría del mundo.
constexpr int kOriginColumn = 0;
constexpr int kOriginRow = 0;
constexpr int kDefaultChunkSizePixels = 32;
constexpr GridSpan kUnitCellSpan{1, 1};

}
