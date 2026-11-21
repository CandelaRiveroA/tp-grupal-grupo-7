#include "world/MarketSideNames.h"

namespace world {

MarketSideNames::MarketSideNames()
	: sides_{
		  {"sells", MarketSide::Sells},
		  {"buys", MarketSide::Buys},
	  } {}

std::optional<MarketSide> MarketSideNames::find(
	const std::string& name) const {
	const auto it = sides_.find(name);
	if (it == sides_.end()) {
		return std::nullopt;
	}
	return it->second;
}

const std::map<std::string, MarketSide>& MarketSideNames::entries() const {
	return sides_;
}

}
