#pragma once

#include "world_model/Geometry.h"

namespace worldmodel {

enum class MarkerKind {
	Bed,
	MerchantMarnie,
	MerchantClint,
	MerchantPierre,
	MerchantRobin,
	GeneralWindow,
	CoopWindow,
};

struct Marker {
	MarkerKind kind = MarkerKind::Bed;
	TilePos cell;
};

}
