#include "world/MarketOfferResolver.h"

#include <map>
#include <set>
#include <string>
#include <utility>

namespace world {

MarketOfferResolver::MarketOfferResolver(const MarketCatalog& catalog)
	: catalog_(catalog) {}

std::vector<MarketSection> MarketOfferResolver::sectionsFrom(
	const MarketOffer& offer) const {
	std::vector<MarketEntry> source = offer.entries;
	if (source.empty()) {
		source = catalog_.defaultEntries(offer.merchantId, 0);
		if (source.empty()) {
			return {};
		}
	}

	std::map<MarketSide, MarketSection> bySide;
	std::set<std::pair<MarketSide, std::string>> seen;
	for (const MarketEntry& entry : source) {
		const std::string* label = catalog_.labelOf(entry.contentId);
		const auto title = catalog_.sideTitles().find(entry.side);
		if (label == nullptr || title == catalog_.sideTitles().end()) {
			continue;
		}
		if (!seen.emplace(entry.side, entry.contentId).second) {
			continue;
		}
		MarketItem item;
		item.contentId = entry.contentId;
		item.label = entry.label.empty() ? *label : entry.label;
		if (entry.price.has_value() && *entry.price >= 0) {
			item.price = entry.price;
		}
		MarketSection& section = bySide[entry.side];
		section.title = title->second;
		section.items.push_back(std::move(item));
	}

	std::vector<MarketSection> sections;
	for (auto& [side, section] : bySide) {
		sections.push_back(std::move(section));
	}
	return sections;
}

}
