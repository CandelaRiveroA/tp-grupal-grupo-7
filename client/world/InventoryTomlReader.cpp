#include "world/InventoryTomlReader.h"

#include <array>
#include <vector>

#include "world/ConfigError.h"

namespace world {

namespace {

using Kind = ConfigErrorKind;

constexpr const char* kSheet = "sheet";
constexpr const char* kSheetImage = "sheet.image";
constexpr const char* kSheetCell = "sheet.cell";
constexpr const char* kSlotCell = "slot.cell";
constexpr const char* kBarCount = "bar.count";
constexpr const char* kBarScale = "bar.scale";
constexpr const char* kBarGap = "bar.gap";
constexpr const char* kBarAnchor = "bar.anchor";
constexpr const char* kBarMargin = "bar.margin";
constexpr const char* kBarUiScale = "bar.ui_scale";
constexpr const char* kBarItemPad = "bar.item_pad";

constexpr const char* kFrame = "frame";
constexpr const char* kSlotFrame = "slot_frame";
constexpr const char* kActiveSlotFrame = "active_slot_frame";

constexpr const char* kBorderCells = "border_cells";
constexpr const char* kThickness = "thickness";
constexpr const char* kInset = "inset";
constexpr const char* kCornersOnly = "corners_only";
constexpr const char* kTopLeft = "top_left";
constexpr const char* kTopRight = "top_right";
constexpr const char* kBottomLeft = "bottom_left";
constexpr const char* kBottomRight = "bottom_right";
constexpr const char* kTop = "top";
constexpr const char* kBottom = "bottom";
constexpr const char* kLeft = "left";
constexpr const char* kRight = "right";

constexpr const char* kAnchorBottomCenter = "bottom_center";

constexpr const char* kItems = "items";
constexpr const char* kItemSheet = "sheet";
constexpr const char* kItemCell = "cell";
constexpr const char* kItemAt = "at";

constexpr const char* kBarDigitScale = "bar.digit_scale";
constexpr const char* kBarDigitMargin = "bar.digit_margin";

}

InventoryTomlReader::InventoryTomlReader(const tomlcfg::TomlDocument& doc)
	: doc_(doc) {}

InventoryLayout InventoryTomlReader::read() const {
	InventoryLayout layout;

	layout.sheetImage = doc_.getString(kSheetImage);
	const std::array<int, 2> cell = doc_.getIntArray<2>(kSheetCell);
	layout.cellWidth = cell[0];
	layout.cellHeight = cell[1];
	if (layout.cellWidth <= 0 || layout.cellHeight <= 0) {
		throw ConfigError(Kind::InvalidValue, kSheetCell);
	}

	const std::array<int, 2> slot = doc_.getIntArray<2>(kSlotCell);
	layout.slotCol = slot[0];
	layout.slotRow = slot[1];
	if (layout.slotCol < 0 || layout.slotRow < 0) {
		throw ConfigError(Kind::InvalidValue, kSlotCell);
	}

	layout.count = doc_.getInt(kBarCount);
	if (layout.count <= 0) {
		throw ConfigError(Kind::InvalidValue, kBarCount);
	}

	layout.scale = doc_.getInt(kBarScale);
	if (layout.scale <= 0) {
		throw ConfigError(Kind::InvalidValue, kBarScale);
	}

	layout.gap = doc_.getInt(kBarGap);
	if (layout.gap < 0) {
		throw ConfigError(Kind::InvalidValue, kBarGap);
	}

	layout.margin = doc_.getInt(kBarMargin);
	if (layout.margin < 0) {
		throw ConfigError(Kind::InvalidValue, kBarMargin);
	}

	layout.anchor = parseAnchor(doc_.getString(kBarAnchor));

	layout.uiScale =
		doc_.hasKey(kBarUiScale) ? doc_.getDouble(kBarUiScale) : 1.0;
	if (layout.uiScale <= 0.0) {
		throw ConfigError(Kind::InvalidValue, kBarUiScale);
	}

	layout.itemPad =
		doc_.hasKey(kBarItemPad) ? doc_.getInt(kBarItemPad) : -1;

	if (doc_.hasKey(kBarDigitScale)) {
		layout.digitScale = doc_.getDouble(kBarDigitScale);
		if (layout.digitScale <= 0.0) {
			throw ConfigError(Kind::InvalidValue, kBarDigitScale);
		}
	}

	if (doc_.hasKey(kBarDigitMargin)) {
		layout.digitMargin = doc_.getInt(kBarDigitMargin);
		if (layout.digitMargin < 0) {
			throw ConfigError(Kind::InvalidValue, kBarDigitMargin);
		}
	}

	if (doc_.hasKey(kFrame)) {
		readFrame(kFrame, layout.frame);
	}

	if (doc_.hasKey(kSlotFrame)) {
		readFrame(kSlotFrame, layout.slotFrame);
	}

	if (doc_.hasKey(kActiveSlotFrame)) {
		readFrame(kActiveSlotFrame, layout.activeSlotFrame);
	}

	if (doc_.hasKey(kItems)) {
		readItems(layout.content);
	}

	return layout;
}

void InventoryTomlReader::readItems(world::ContentCatalog& catalog) const {
	const std::vector<std::string> names = doc_.subtableKeys(kItems);
	for (const std::string& name : names) {
		const std::string base = join(kItems, name);
		world::ItemVisual visual;
		visual.sheetImage = doc_.getString(join(base, kItemSheet));
		const std::array<int, 2> cell =
			doc_.getIntArray<2>(join(base, kItemCell));
		visual.cellWidth = cell[0];
		visual.cellHeight = cell[1];
		const std::array<int, 2> at =
			doc_.getIntArray<2>(join(base, kItemAt));
		visual.col = at[0];
		visual.row = at[1];
		if (visual.cellWidth <= 0 || visual.cellHeight <= 0 ||
		    visual.col < 0 || visual.row < 0) {
			throw ConfigError(Kind::InvalidValue, base);
		}
		// contentId = nombre de la subtabla. No se traduce a ningún
		// enum: el inventario es genérico.
		catalog.visuals.emplace(name, visual);
	}
}

void InventoryTomlReader::readFrame(const std::string& section,
				    world::FrameCells& frame) const {
	frame.enabled = true;

	frame.borderCells = doc_.getInt(join(section, kBorderCells));
	if (frame.borderCells <= 0) {
		throw ConfigError(Kind::InvalidValue,
				  join(section, kBorderCells));
	}

	frame.thickness = doc_.getInt(join(section, kThickness));
	if (frame.thickness <= 0) {
		throw ConfigError(Kind::InvalidValue,
				  join(section, kThickness));
	}

	const std::string insetKey = join(section, kInset);
	frame.inset = doc_.hasKey(insetKey) ? doc_.getInt(insetKey) : 0;
	if (frame.inset < 0) {
		throw ConfigError(Kind::InvalidValue, insetKey);
	}

	const std::string cornersKey = join(section, kCornersOnly);
	frame.cornersOnly =
		doc_.hasKey(cornersKey) ? doc_.getBool(cornersKey) : false;

	frame.topLeft = readCell(join(section, kTopLeft));
	frame.topRight = readCell(join(section, kTopRight));
	frame.bottomLeft = readCell(join(section, kBottomLeft));
	frame.bottomRight = readCell(join(section, kBottomRight));

	if (!frame.cornersOnly) {
		frame.top = readCell(join(section, kTop));
		frame.bottom = readCell(join(section, kBottom));
		frame.left = readCell(join(section, kLeft));
		frame.right = readCell(join(section, kRight));
	}
}

std::string InventoryTomlReader::join(const std::string& section,
				      const std::string& key) const {
	return section + "." + key;
}

SheetCell InventoryTomlReader::readCell(const std::string& key) const {
	const std::array<int, 2> cell = doc_.getIntArray<2>(key);
	if (cell[0] < 0 || cell[1] < 0) {
		throw ConfigError(Kind::InvalidValue, key);
	}
	return SheetCell{cell[0], cell[1]};
}

InventoryAnchor InventoryTomlReader::parseAnchor(
	const std::string& anchor) const {
	if (anchor == kAnchorBottomCenter) {
		return InventoryAnchor::BottomCenter;
	}
	throw ConfigError(Kind::InvalidValue, kBarAnchor);
}

}
