#include "world/MarketCatalog.h"

#include <algorithm>
#include <array>
#include <string>
#include <utility>

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"

namespace world {

MarketCatalog::MarketCatalog(const std::string& path) {
	try {
		const tomlcfg::TomlDocument doc(
			path, tomlcfg::TomlDocument::Source::File);
		readStyle(doc);
		readSides(doc);
		readQualities(doc);
		readProducts(doc);
		readMerchants(doc);
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

const MarketPopupStyle& MarketCatalog::style() const { return style_; }

const std::map<MarketSide, std::string>& MarketCatalog::sideTitles() const {
	return sideTitles_;
}

const std::string* MarketCatalog::labelOf(
	const std::string& contentId) const {
	const auto found = labels_.find(contentId);
	return found != labels_.end() ? &found->second : nullptr;
}

const MarketQualityStyle* MarketCatalog::qualityOf(
	const CropQuality quality) const {
	const auto found = qualities_.find(quality);
	return found != qualities_.end() ? &found->second : nullptr;
}

std::vector<MarketEntry> MarketCatalog::defaultEntries(
	const std::string& npcId, int set) const {
	std::vector<MarketEntry> entries;
	const auto found = defaults_.find(npcId);
	if (found == defaults_.end()) {
		return entries;
	}
	for (const DefaultEntry& candidate : found->second) {
		if (candidate.sets.empty() ||
		    std::find(candidate.sets.begin(), candidate.sets.end(),
			      set) != candidate.sets.end()) {
			entries.push_back(candidate.entry);
		}
	}
	return entries;
}

int MarketCatalog::defaultSetCount(const std::string& npcId) const {
	int count = 1;
	const auto found = defaults_.find(npcId);
	if (found == defaults_.end()) {
		return count;
	}
	for (const DefaultEntry& candidate : found->second) {
		for (const int set : candidate.sets) {
			count = std::max(count, set + 1);
		}
	}
	return count;
}

void MarketCatalog::readStyle(const tomlcfg::TomlDocument& doc) {
	const std::string base = "popup.";
	const std::array<int, 4> title =
		doc.getIntArray<4>(base + "title_color");
	const std::array<int, 4> text =
		doc.getIntArray<4>(base + "text_color");
	const std::array<int, 4> price =
		doc.getIntArray<4>(base + "price_color");

	style_.panelColumns = doc.getInt(base + "panel_columns");
	style_.inset = doc.getInt(base + "inset");
	style_.columns = doc.getInt(base + "columns");
	style_.titleHeight = doc.getInt(base + "title_height");
	style_.sectionGap = doc.getInt(base + "section_gap");
	style_.rowHeight = doc.getInt(base + "row_height");
	style_.priceHeight = doc.getInt(base + "price_height");
	style_.iconSize = doc.getInt(base + "icon_size");
	style_.iconGap = doc.getInt(base + "icon_gap");
	style_.labelOffsetY = doc.getInt(base + "label_offset_y");
	style_.coinGap = doc.getInt(base + "coin_gap");
	style_.coin = SpriteVisual(doc, "popup.coin", "at");
	style_.headerHeight = doc.getInt(base + "header_height");
	style_.tierHeight = doc.getInt(base + "tier_height");
	style_.tierPriceLeft = doc.getInt(base + "tier_price_left");
	style_.starGap = doc.getInt(base + "star_gap");
	style_.star = SpriteVisual(doc, "popup.star");
	style_.titleSize = doc.getInt(base + "title_size");
	style_.textSize = doc.getInt(base + "text_size");
	style_.titleColor = Color{title[0], title[1], title[2], title[3]};
	style_.textColor = Color{text[0], text[1], text[2], text[3]};
	style_.priceColor = Color{price[0], price[1], price[2], price[3]};

	const auto invalid = [&](const std::string& key) {
		throw ConfigError(ConfigErrorKind::InvalidValue, base + key);
	};
	if (style_.panelColumns <= 0) {
		invalid("panel_columns");
	}
	if (style_.columns <= 0) {
		invalid("columns");
	}
	if (style_.inset < 0) {
		invalid("inset");
	}
	if (style_.titleHeight < 0 || style_.sectionGap < 0 ||
	    style_.priceHeight < 0 || style_.iconGap < 0 ||
	    style_.labelOffsetY < 0 || style_.coinGap < 0) {
		invalid("title_height");
	}
	if (style_.rowHeight <= 0) {
		invalid("row_height");
	}
	if (style_.headerHeight <= 0) {
		invalid("header_height");
	}
	if (style_.tierHeight <= 0) {
		invalid("tier_height");
	}
	if (style_.tierPriceLeft < 0 || style_.starGap < 0) {
		invalid("tier_price_left");
	}
	if (style_.iconSize <= 0) {
		invalid("icon_size");
	}
	if (style_.titleSize <= 0 || style_.textSize <= 0) {
		invalid("text_size");
	}
}

void MarketCatalog::readSides(const tomlcfg::TomlDocument& doc) {
	for (const auto& [name, side] : sideNames_.entries()) {
		const std::string path = "sides." + name;
		const std::string title = doc.getString(path);
		if (title.empty()) {
			throw ConfigError(ConfigErrorKind::InvalidValue, path);
		}
		sideTitles_.emplace(side, title);
	}
}

void MarketCatalog::readQualities(const tomlcfg::TomlDocument& doc) {
	for (const auto& [name, quality] : qualityNames_.entries()) {
		const std::string base = "qualities." + name;
		MarketQualityStyle style;
		style.label = doc.getString(base + ".label");
		if (style.label.empty()) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  base + ".label");
		}
		style.multiplier = doc.getDouble(base + ".multiplier");
		if (style.multiplier <= 0.0) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  base + ".multiplier");
		}
		// La estrella es opcional: la calidad base no lleva.
		if (doc.hasKey(base + ".star")) {
			style.star = SheetCell(doc, base + ".star");
		}
		qualities_.emplace(quality, std::move(style));
	}
}

void MarketCatalog::readProducts(const tomlcfg::TomlDocument& doc) {
	const std::vector<tomlcfg::TomlDocument> products =
		doc.getTableArray("products");
	if (products.empty()) {
		throw ConfigError(ConfigErrorKind::MissingStructure,
				  "products");
	}
	for (std::size_t i = 0; i < products.size(); ++i) {
		const std::string base = "products[" + std::to_string(i) + "]";
		const std::string contentId = products[i].getString("content_id");
		const std::string label = products[i].getString("label");
		if (contentId.empty() || labels_.count(contentId) > 0) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  base + ".content_id");
		}
		if (label.empty()) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  base + ".label");
		}
		labels_.emplace(contentId, label);
	}
}

void MarketCatalog::readMerchants(const tomlcfg::TomlDocument& doc) {
	const std::vector<tomlcfg::TomlDocument> merchants =
		doc.getTableArray("merchants");
	if (merchants.empty()) {
		throw ConfigError(ConfigErrorKind::MissingStructure,
				  "merchants");
	}
	for (std::size_t m = 0; m < merchants.size(); ++m) {
		const std::string base = "merchants[" + std::to_string(m) + "]";
		const std::string npc = merchants[m].getString("npc");
		if (npc.empty() || defaults_.count(npc) > 0) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  base + ".npc");
		}
		const std::vector<tomlcfg::TomlDocument> offer =
			merchants[m].getTableArray("offer");
		if (offer.empty()) {
			throw ConfigError(ConfigErrorKind::MissingStructure,
					  base + ".offer");
		}
		std::vector<DefaultEntry>& target = defaults_[npc];
		for (std::size_t i = 0; i < offer.size(); ++i) {
			for (DefaultEntry& entry : readEntries(
				     offer[i], base + ".offer[" +
						       std::to_string(i) + "]")) {
				target.push_back(std::move(entry));
			}
		}
	}
}

std::vector<MarketCatalog::DefaultEntry> MarketCatalog::readEntries(
	const tomlcfg::TomlDocument& doc, const std::string& base) const {
	DefaultEntry plain = readEntry(doc, base);
	if (!doc.hasKey("qualities") || !doc.getBool("qualities")) {
		return {plain};
	}
	// Una entrada por calidad declarada, con el monto base escalado por
	// su multiplicador (truncado, como el servidor).
	std::vector<DefaultEntry> expanded;
	for (const auto& [quality, style] : qualities_) {
		DefaultEntry variant = plain;
		variant.entry.quality = quality;
		if (plain.entry.price.has_value()) {
			variant.entry.price = static_cast<int>(
				*plain.entry.price * style.multiplier);
		}
		expanded.push_back(std::move(variant));
	}
	return expanded;
}

MarketCatalog::DefaultEntry MarketCatalog::readEntry(
	const tomlcfg::TomlDocument& doc, const std::string& base) const {
	DefaultEntry result;
	MarketEntry& entry = result.entry;
	entry.contentId = doc.getString("content_id");
	const std::optional<MarketSide> side =
		sideNames_.find(doc.getString("side"));
	if (!side.has_value()) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  base + ".side");
	}
	entry.side = *side;
	if (labels_.count(entry.contentId) == 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  base + ".content_id");
	}
	if (doc.hasKey("price")) {
		entry.price = doc.getInt("price");
		if (*entry.price < 0) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  base + ".price");
		}
	}
	// Estilos rotativos en que aparece (opcional; sin `sets` = todos).
	if (doc.hasKey("sets")) {
		const std::size_t count = doc.arraySize("sets");
		for (std::size_t i = 0; i < count; ++i) {
			const int set =
				doc.getInt("sets[" + std::to_string(i) + "]");
			if (set < 0) {
				throw ConfigError(ConfigErrorKind::InvalidValue,
						  base + ".sets");
			}
			result.sets.push_back(set);
		}
	}
	return result;
}

}
