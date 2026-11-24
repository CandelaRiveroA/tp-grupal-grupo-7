#include "world/CommunityCatalog.h"

#include <algorithm>
#include <array>
#include <string>
#include <utility>

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"

namespace world {

CommunityCatalog::CommunityCatalog(const std::string& path) {
	try {
		const tomlcfg::TomlDocument doc(
			path, tomlcfg::TomlDocument::Source::File);
		readText(doc);
		readStyle(doc);
		readSets(doc);
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

const CommunityPopupStyle& CommunityCatalog::style() const { return style_; }

const std::string& CommunityCatalog::title() const { return title_; }

const std::string& CommunityCatalog::status() const { return status_; }

const std::vector<CommunitySetDef>& CommunityCatalog::sets() const {
	return sets_;
}

void CommunityCatalog::readText(const tomlcfg::TomlDocument& doc) {
	title_ = doc.getString("text.title");
	status_ = doc.getString("text.status");
	if (title_.empty()) {
		throw ConfigError(ConfigErrorKind::InvalidValue, "text.title");
	}
	if (status_.empty()) {
		throw ConfigError(ConfigErrorKind::InvalidValue, "text.status");
	}
}

void CommunityCatalog::readStyle(const tomlcfg::TomlDocument& doc) {
	const std::string base = "popup.";
	const auto color = [&](const std::string& key) {
		const std::array<int, 4> rgba = doc.getIntArray<4>(base + key);
		return Color{rgba[0], rgba[1], rgba[2], rgba[3]};
	};

	style_.panelColumns = doc.getInt(base + "panel_columns");
	style_.inset = doc.getInt(base + "inset");
	style_.titleHeight = doc.getInt(base + "title_height");
	style_.headerHeight = doc.getInt(base + "header_height");
	style_.rowHeight = doc.getInt(base + "row_height");
	style_.setGap = doc.getInt(base + "set_gap");
	style_.checkboxGap = doc.getInt(base + "checkbox_gap");
	style_.statusGap = doc.getInt(base + "status_gap");
	style_.amountWidth = doc.getInt(base + "amount_width");
	style_.titleSize = doc.getInt(base + "title_size");
	style_.headerSize = doc.getInt(base + "header_size");
	style_.textSize = doc.getInt(base + "text_size");
	style_.titleColor = color("title_color");
	style_.headerColor = color("header_color");
	style_.textColor = color("text_color");
	style_.amountColor = color("amount_color");
	style_.checkboxEmpty = SpriteVisual(doc, "popup.checkbox_empty", "at");
	style_.checkboxDone = SpriteVisual(doc, "popup.checkbox_done", "at");
	style_.setDone = SpriteVisual(doc, "popup.set_done", "at");

	const auto invalid = [&](const std::string& key) {
		throw ConfigError(ConfigErrorKind::InvalidValue, base + key);
	};
	if (style_.panelColumns <= 0) {
		invalid("panel_columns");
	}
	if (style_.rowHeight <= 0) {
		invalid("row_height");
	}
	if (style_.headerHeight <= 0) {
		invalid("header_height");
	}
	if (style_.amountWidth <= 0) {
		invalid("amount_width");
	}
	if (style_.inset < 0 || style_.titleHeight < 0 || style_.setGap < 0 ||
	    style_.checkboxGap < 0 || style_.statusGap < 0) {
		invalid("inset");
	}
	if (style_.titleSize <= 0 || style_.headerSize <= 0 ||
	    style_.textSize <= 0) {
		invalid("text_size");
	}
}

void CommunityCatalog::readSets(const tomlcfg::TomlDocument& doc) {
	const std::vector<tomlcfg::TomlDocument> sets = doc.getTableArray("sets");
	if (sets.empty()) {
		throw ConfigError(ConfigErrorKind::MissingStructure, "sets");
	}
	for (std::size_t i = 0; i < sets.size(); ++i) {
		const std::string base = "sets[" + std::to_string(i) + "]";
		CommunitySetDef set = readSet(sets[i], base);
		const bool repeated = std::any_of(
			sets_.begin(), sets_.end(),
			[&](const CommunitySetDef& other) {
				return other.id == set.id;
			});
		if (repeated) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  base + ".id");
		}
		sets_.push_back(std::move(set));
	}
}

CommunitySetDef CommunityCatalog::readSet(const tomlcfg::TomlDocument& doc,
					  const std::string& base) const {
	CommunitySetDef set;
	set.id = doc.getInt("id");
	set.title = doc.getString("title");
	if (set.id <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue, base + ".id");
	}
	if (set.title.empty()) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  base + ".title");
	}
	if (doc.hasKey("amount_prefix")) {
		set.amountPrefix = doc.getString("amount_prefix");
	}
	const std::vector<tomlcfg::TomlDocument> goals =
		doc.getTableArray("goals");
	if (goals.empty()) {
		throw ConfigError(ConfigErrorKind::MissingStructure,
				  base + ".goals");
	}
	for (std::size_t i = 0; i < goals.size(); ++i) {
		const std::string goalBase =
			base + ".goals[" + std::to_string(i) + "]";
		CommunityGoalDef goal = readGoal(goals[i], goalBase);
		const bool repeated = std::any_of(
			set.goals.begin(), set.goals.end(),
			[&](const CommunityGoalDef& other) {
				return other.id == goal.id;
			});
		if (repeated) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  goalBase + ".id");
		}
		set.goals.push_back(std::move(goal));
	}
	return set;
}

CommunityGoalDef CommunityCatalog::readGoal(const tomlcfg::TomlDocument& doc,
					    const std::string& base) const {
	CommunityGoalDef goal;
	goal.id = doc.getString("id");
	if (goal.id.empty()) {
		throw ConfigError(ConfigErrorKind::InvalidValue, base + ".id");
	}
	if (doc.hasKey("label")) {
		goal.label = doc.getString("label");
	}
	return goal;
}

}
