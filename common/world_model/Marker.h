#pragma once

#include "world_model/Geometry.h"

namespace worldmodel {

enum class MarkerKind {
	PlayerSpawn,
	Bed,
	MerchantMarnie,
	MerchantClint,
	MerchantPierre,
	MerchantRobin,
};

struct Marker {
	MarkerKind kind = MarkerKind::PlayerSpawn;
	TilePos cell;
};

}
