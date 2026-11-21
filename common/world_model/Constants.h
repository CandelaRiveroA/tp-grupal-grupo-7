#pragma once

#include "world_model/Geometry.h"

namespace worldmodel {

// Limites de cupo y estructura fija del mundo.
constexpr int kMinPlayers = 1;
constexpr int kMaxPlayers = 4;
constexpr int kDungeonLevels = 10;
constexpr int kBaseMandatoryMaps = 4;  // farm, town, market, community_center

// Layout y tamanos fallback de edificios en WorldFactory cuando no hay prefab.
constexpr TilePos kFarmCabinOrigin{2, 2};
constexpr int kFarmCabinGap = 2;
constexpr int kFallbackCabinWidth = 4;
constexpr int kFallbackCabinHeight = 3;
constexpr TilePos kFallbackCabinDoor{2, 3};

constexpr int kFallbackMarketWidth = 5;
constexpr int kFallbackMarketHeight = 4;
constexpr TilePos kFallbackMarketDoor{2, 4};

constexpr int kFallbackCenterWidth = 6;
constexpr int kFallbackCenterHeight = 4;
constexpr TilePos kFallbackCenterDoor{3, 4};

}  // namespace worldmodel
