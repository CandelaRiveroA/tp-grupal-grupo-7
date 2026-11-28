#include "world/CommunityCatalog.h"

#include <unordered_set>
#include <utility>

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"
#include "world/TomlConfigReader.h"

namespace world {

CommunityCatalog::CommunityCatalog(const std::string& path) {
	try {
		const tomlcfg::TomlDocument doc(
			path, tomlcfg::TomlDocument::Source::File);
		const TomlConfigReader root(doc);
		readText(root);
		readStyle(root);
		readSets(root);
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

const CommunityPopupStyle& CommunityCatalog::style() const { return style_; }

const std::string& CommunityCatalog::title() const { return title_; }

const std::string& CommunityCatalog::status() const { return status_; }

const std::string& CommunityCatalog::donateLabel() const {
	return donateLabel_;
}

const std::string& CommunityCatalog::helpCloseHint() const {
	return helpCloseHint_;
}

const std::vector<CommunitySetDef>& CommunityCatalog::sets() const {
	return sets_;
}

void CommunityCatalog::readText(const TomlConfigReader& root) {
	const TomlConfigReader text = root.subtable("text");
	title_ = text.nonEmptyString("title");
	status_ = text.nonEmptyString("status");
	donateLabel_ = text.nonEmptyString("donate_label");
	helpCloseHint_ = text.nonEmptyString("help_close_hint");
}

void CommunityCatalog::readStyle(const TomlConfigReader& root) {
	style_.modal = PagedModalSpec(root.document());

	const TomlConfigReader popup = root.subtable("popup");

	CommunityTopBarStyle& topBar = style_.topBar;
	topBar.superTitleTop = popup.nonNegativeInt("super_title_top");
	topBar.superTitleHeight = popup.positiveInt("super_title_height");
	topBar.superTitleSize = popup.positiveInt("super_title_size");
	topBar.pageCounterWidth = popup.positiveInt("page_counter_width");
	topBar.pageSize = popup.positiveInt("page_size");

	CommunityHeaderStyle& header = style_.header;
	header.gap = popup.nonNegativeInt("header_gap");
	header.height = popup.positiveInt("header_height");
	header.titleSize = popup.positiveInt("header_title_size");
	header.statusSize = popup.positiveInt("header_status_size");
	header.statusGap = popup.nonNegativeInt("status_gap");

	CommunityCardStyle& card = style_.card;
	card.contentGap = popup.nonNegativeInt("content_gap");
	card.height = popup.positiveInt("card_height");
	card.gap = popup.nonNegativeInt("card_gap");
	card.padX = popup.nonNegativeInt("card_pad_x");
	card.padY = popup.nonNegativeInt("card_pad_y");
	card.border = popup.nonNegativeInt("card_border");
	card.iconColumnWidth = popup.positiveInt("icon_column_width");
	card.spriteSize = popup.positiveInt("sprite_size");
	card.creatureSpriteSize = popup.positiveInt("creature_sprite_size");
	card.spriteGap = popup.nonNegativeInt("sprite_gap");
	card.qualitySlotSize = popup.positiveInt("quality_slot_size");
	card.qualitySlotGap = popup.nonNegativeInt("quality_slot_gap");
	card.qualitySlotPad = popup.nonNegativeInt("quality_slot_pad");
	card.qualitySlotBorder = popup.nonNegativeInt("quality_slot_border");
	card.barHeight = popup.positiveInt("bar_height");
	card.barBorder = popup.nonNegativeInt("bar_border");
	card.barTopGap = popup.nonNegativeInt("bar_top_gap");
	card.amountWidth = popup.positiveInt("amount_width");
	card.checkboxGap = popup.nonNegativeInt("checkbox_gap");
	card.labelSize = popup.positiveInt("label_size");
	card.amountSize = popup.positiveInt("amount_size");

	CommunityFooterStyle& footer = style_.footer;
	footer.height = popup.positiveInt("footer_height");
	footer.margin = popup.nonNegativeInt("footer_margin");
	footer.donateGap = popup.nonNegativeInt("donate_gap");
	footer.dotSize = popup.positiveInt("dot_size");
	footer.dotGap = popup.nonNegativeInt("dot_gap");
	footer.dotBorder = popup.nonNegativeInt("dot_border");
	footer.donateTextSize = popup.positiveInt("donate_text_size");
	const PixelSize donateSize = popup.positiveSize("donate_button_size");
	footer.donateButtonWidth = donateSize.width;
	footer.donateButtonHeight = donateSize.height;
	footer.donateButtonSheet = popup.nonEmptyString("donate_button_sheet");

	CommunityPopupColors& colors = style_.colors;
	colors.inkMuted = popup.color("ink_muted");
	colors.cardBg = popup.color("card_bg");
	colors.cardDoneBg = popup.color("card_done_bg");
	colors.spriteSlotBg = popup.color("sprite_slot_bg");
	colors.barTrack = popup.color("bar_track");
	colors.barFill = popup.color("bar_fill");
	colors.amountColor = popup.color("amount_color");
	colors.donateTextColor = popup.color("donate_text_color");

	readHelpStyle(popup);

	style_.sprites.checkboxEmpty =
		SpriteVisual(root.document(), "popup.checkbox_empty", "at");
	style_.sprites.checkboxDone =
		SpriteVisual(root.document(), "popup.checkbox_done", "at");
	style_.sprites.setDone =
		SpriteVisual(root.document(), "popup.set_done", "at");
	style_.sprites.coin = SpriteVisual(root.document(), "popup.coin", "at");
}

void CommunityCatalog::readHelpStyle(const TomlConfigReader& popup) {
	const TomlConfigReader help = popup.subtable("help");
	HelpPopupStyle& h = style_.help;

	h.buttonSheet = help.nonEmptyString("button_sheet");
	h.buttonFallback = help.nonEmptyString("button_fallback");
	const PixelSize btnSize = help.positiveSize("button_size");
	h.buttonWidth = btnSize.width;
	h.buttonHeight = btnSize.height;
	h.buttonGap = help.nonNegativeInt("button_gap");

	const PixelSize boxSize = help.positiveSize("popup_size");
	h.popupWidth = boxSize.width;
	h.popupHeight = boxSize.height;
	h.pad = help.nonNegativeInt("pad");
	h.border = help.nonNegativeInt("border");
	h.titleHeight = help.positiveInt("title_height");
	h.titleSize = help.positiveInt("title_size");
	h.titleGap = help.nonNegativeInt("title_gap");
	h.textSize = help.positiveInt("text_size");
	h.lineGap = help.nonNegativeInt("line_gap");
	h.hintHeight = help.positiveInt("hint_height");
	h.hintSize = help.positiveInt("hint_size");
	h.backdrop = help.color("backdrop");
}

void CommunityCatalog::readSets(const TomlConfigReader& root) {
	const std::vector<TomlConfigReader> setReaders =
		root.tableArray("sets");
	std::unordered_set<int> seenIds;
	sets_.reserve(setReaders.size());
	for (const TomlConfigReader& setReader : setReaders) {
		CommunitySetDef set = readSet(setReader);
		if (!seenIds.insert(set.id).second) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  setReader.pathFor("id"));
		}
		sets_.push_back(std::move(set));
	}
}

CommunitySetDef CommunityCatalog::readSet(
	const TomlConfigReader& setReader) const {
	CommunitySetDef set;
	set.id = setReader.positiveInt("id");
	set.title = setReader.nonEmptyString("title");
	set.helpText = setReader.nonEmptyString("help");
	set.donatable = setReader.boolOr("donatable", true);

	const std::vector<TomlConfigReader> goalReaders =
		setReader.tableArray("goals");
	std::unordered_set<std::string> seenGoalIds;
	set.goals.reserve(goalReaders.size());
	for (const TomlConfigReader& goalReader : goalReaders) {
		CommunityGoalDef goal = readGoal(goalReader);
		if (!seenGoalIds.insert(goal.id).second) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  goalReader.pathFor("id"));
		}
		set.goals.push_back(std::move(goal));
	}
	return set;
}

CommunityGoalDef CommunityCatalog::readGoal(
	const TomlConfigReader& goalReader) const {
	CommunityGoalDef goal;
	goal.id = goalReader.nonEmptyString("id");
	goal.label = goalReader.nonEmptyString("label");

	const bool hasIcon = goalReader.hasKey("icon");
	const bool hasCreature = goalReader.hasKey("creature");
	const bool useCoin = goalReader.boolOr("coin", false);

	if (hasIcon) {
		goal.visual.iconId = goalReader.nonEmptyString("icon");
	}
	if (hasCreature) {
		const std::string name = goalReader.nonEmptyString("creature");
		const std::optional<CreatureKind> found =
			creatureNames_.find(name);
		if (!found.has_value()) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  goalReader.pathFor("creature"));
		}
		goal.visual.creature = found;
	}

	const int primarySources = (hasIcon ? 1 : 0) + (hasCreature ? 1 : 0) +
				   (useCoin ? 1 : 0);
	if (primarySources > 1) {
		const std::string conflictKey =
			useCoin ? "coin" : "creature";
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  goalReader.pathFor(conflictKey));
	}

	if (goalReader.hasKey("qualities")) {
		const std::vector<std::string> names =
			goalReader.stringArray("qualities");
		if (names.empty() || !hasIcon || hasCreature || useCoin) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  goalReader.pathFor("qualities"));
		}
		for (const std::string& name : names) {
			const std::optional<CropQuality> quality =
				qualityNames_.find(name);
			if (!quality.has_value()) {
				throw ConfigError(
					ConfigErrorKind::InvalidValue,
					goalReader.pathFor("qualities"));
			}
			goal.visual.cropQualities.push_back(*quality);
		}
	}

	if (!goal.visual.cropQualities.empty()) {
		goal.visual.kind = CommunityVisualKind::CropQualities;
	} else if (hasIcon) {
		goal.visual.kind = CommunityVisualKind::ContentIcon;
	} else if (hasCreature) {
		goal.visual.kind = CommunityVisualKind::Creature;
	} else if (useCoin) {
		goal.visual.kind = CommunityVisualKind::Coin;
	}

	if (goalReader.hasKey("sprite_size")) {
		goal.visual.spriteSize = goalReader.positiveInt("sprite_size");
	}
	goal.visual.spriteOffsetY = goalReader.intOr("sprite_offset_y", 0);
	return goal;
}

}  // namespace world
