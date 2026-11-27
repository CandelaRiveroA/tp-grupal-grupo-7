#include "world/MarketOfferResolver.h"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <tuple>
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
	// Un producto es una sola celda aunque traiga varias calidades: se
	// ubica por (lado, contentId) dentro de su sección.
	std::map<std::pair<MarketSide, std::string>, std::size_t> position;
	std::set<std::tuple<MarketSide, std::string,
			    std::optional<CropQuality>>> seen;
	for (const MarketEntry& entry : source) {
		const std::string* label = catalog_.labelOf(entry.contentId);
		const auto title = catalog_.sideTitles().find(entry.side);
		if (label == nullptr || title == catalog_.sideTitles().end()) {
			continue;
		}
		std::optional<MarketTier> tier;
		if (entry.quality.has_value()) {
			tier = tierOf(entry);
			if (!tier.has_value()) {
				continue;
			}
		}
		if (!seen.emplace(entry.side, entry.contentId, entry.quality)
			     .second) {
			continue;
		}
		MarketSection& section = bySide[entry.side];
		section.title = title->second;
		const auto key = std::make_pair(entry.side, entry.contentId);
		auto where = position.find(key);
		if (where == position.end()) {
			MarketItem item;
			item.contentId = entry.contentId;
			item.label = entry.label.empty() ? *label : entry.label;
			where = position.emplace(key, section.items.size()).first;
			section.items.push_back(std::move(item));
		}
		MarketItem& item = section.items[where->second];
		if (tier.has_value()) {
			item.tiers.push_back(std::move(*tier));
		} else if (entry.price.has_value() && *entry.price >= 0) {
			item.price = entry.price;
		}
	}

	std::vector<MarketSection> sections;
	for (auto& [side, section] : bySide) {
		for (MarketItem& item : section.items) {
			// Las calidades salen de menor a mayor, vengan como vengan.
			std::stable_sort(item.tiers.begin(), item.tiers.end(),
					 [](const MarketTier& a,
					    const MarketTier& b) {
						 return a.quality < b.quality;
					 });
		}
		sections.push_back(std::move(section));
	}
	return sections;
}

std::optional<MarketTier> MarketOfferResolver::tierOf(
	const MarketEntry& entry) const {
	const MarketQualityStyle* style = catalog_.qualityOf(*entry.quality);
	if (style == nullptr) {
		return std::nullopt;
	}
	MarketTier tier;
	tier.quality = *entry.quality;
	tier.label = style->label;
	tier.star = style->star;
	if (entry.price.has_value() && *entry.price >= 0) {
		tier.price = entry.price;
	}
	return tier;
}

}
