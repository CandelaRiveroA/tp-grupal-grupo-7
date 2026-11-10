#include "world/FarmCatalogs.h"

#include <utility>

#include "world/BuildPlaceConfigLoader.h"
#include "world/CropConfigLoader.h"
#include "world/GroundConfigLoader.h"

namespace world {

FarmCatalogs::FarmCatalogs(const std::string& cropsPath,
			   const std::string& groundPath,
			   const std::string& buildPlacePath)
	: FarmCatalogs(CropConfigLoader().loadFromFile(cropsPath),
		       GroundConfigLoader().loadFromFile(groundPath),
		       BuildPlaceConfigLoader().loadFromFile(buildPlacePath)) {}

FarmCatalogs::FarmCatalogs(CropCatalog crops, GroundCatalog ground,
			   BuildPlaceCatalog buildPlaces)
	: crops_(std::move(crops))
	, ground_(std::move(ground))
	, buildPlaces_(std::move(buildPlaces)) {}

InventoryLayout FarmCatalogs::withCropContents(
	const InventoryLayout& base) const {
	InventoryLayout layout = base;
	crops_.registerContents(layout.content, layout.traits);
	return layout;
}

const CropCatalog& FarmCatalogs::crops() const {
	return crops_;
}

const GroundCatalog& FarmCatalogs::ground() const {
	return ground_;
}

const BuildPlaceCatalog& FarmCatalogs::buildPlaces() const {
	return buildPlaces_;
}

}
