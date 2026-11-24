#include "world/CropQualityNames.h"

namespace world {

CropQualityNames::CropQualityNames()
	: qualities_{
		  {"base", CropQuality::Base},
		  {"good", CropQuality::Good},
		  {"excellent", CropQuality::Excellent},
		  {"incredible", CropQuality::Incredible},
		  {"perfect", CropQuality::Perfect},
	  } {}

std::optional<CropQuality> CropQualityNames::find(
	const std::string& name) const {
	const auto it = qualities_.find(name);
	if (it == qualities_.end()) {
		return std::nullopt;
	}
	return it->second;
}

const std::map<std::string, CropQuality>& CropQualityNames::entries() const {
	return qualities_;
}

}
