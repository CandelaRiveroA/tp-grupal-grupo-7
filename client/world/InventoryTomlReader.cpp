#include "world/InventoryTomlReader.h"

#include <array>
#include <utility>
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

constexpr const char* kTrash = "trash";
constexpr const char* kTrashImage = "trash.image";
constexpr const char* kTrashCell = "trash.cell";
constexpr const char* kTrashSize = "trash.size";
constexpr const char* kTrashGap = "trash.gap";
constexpr const char* kTrashIconPad = "trash.icon_pad";
constexpr const char* kTrashHoverTint = "trash.hover_tint";

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
constexpr const char* kItemAt = "at";
constexpr const char* kItemViewSide = "side";
constexpr const char* kItemViewBack = "back";
constexpr std::array<const char*, 2> kItemViews = {kItemViewSide,
						   kItemViewBack};
constexpr const char* kSlotDrawn = "slot_drawn";
constexpr const char* kDroppable = "droppable";

constexpr const char* kBarDigitScale = "bar.digit_scale";
constexpr const char* kBarDigitMargin = "bar.digit_margin";
constexpr const char* kBarDigitMarginX = "bar.digit_margin_x";
constexpr const char* kBarDigitGap = "bar.digit_gap";

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
	}

	// digit_margin_x: margen solo para el eje X, admite negativos para
	// empujar el número hacia afuera del borde derecho del slot.
	// Si no se declara, countBoxes usa digitMargin como fallback.
	if (doc_.hasKey(kBarDigitMarginX)) {
		layout.digitMarginX = doc_.getInt(kBarDigitMarginX);
	}

	// digit_gap: separación extra entre dígitos (admite negativos para
	// solapar y compensar el padding interno del sprite).
	if (doc_.hasKey(kBarDigitGap)) {
		layout.digitGap = doc_.getInt(kBarDigitGap);
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
		readItems(layout);
	}

	if (doc_.hasKey(kTrash)) {
		readTrash(layout);
	}

	return layout;
}

void InventoryTomlReader::readItems(world::InventoryLayout& layout) const {
	const std::vector<std::string> names = doc_.subtableKeys(kItems);
	for (const std::string& name : names) {
		const std::string base = join(kItems, name);
		// contentId = nombre de la subtabla. No se traduce a ningún
		// enum: el inventario es genérico. El sprite se lee completo
		// con el lector compartido (sheet/cell/at/drawn).
		layout.content.visuals.emplace(
			name, world::SpriteVisual(doc_, base, kItemAt));
		// Vistas opcionales (costado, atrás): contenido propio
		// `<id>_<vista>`, sin rasgos.
		for (const char* view : kItemViews) {
			const std::string viewBase = join(base, view);
			if (doc_.hasKey(viewBase)) {
				layout.content.visuals.emplace(
					name + "_" + view,
					world::SpriteVisual(doc_, viewBase,
							    kItemAt));
			}
		}
		// droppable (opcional, default false): rasgo del contenido
		// (¿se puede soltar del inventario?). Vive en la tabla de
		// rasgos, no en la apariencia.
		const std::string droppablePath = join(base, kDroppable);
		layout.traits.set(name,
				  world::ContentTrait{doc_.hasKey(droppablePath) &&
						      doc_.getBool(
							      droppablePath)});
		// slot_drawn (opcional): el tamaño en la celda del
		// inventario, que puede diferir del `drawn` del mapa.
		const std::string slotPath = join(base, kSlotDrawn);
		if (doc_.hasKey(slotPath)) {
			const std::array<int, 2> size =
				doc_.getIntArray<2>(slotPath);
			if (size[0] <= 0 || size[1] <= 0) {
				throw ConfigError(Kind::InvalidValue, slotPath);
			}
			layout.slotDrawn.emplace(name, PixelSize{size[0], size[1]});
		}
	}
}

void InventoryTomlReader::readTrash(world::InventoryLayout& layout) const {
	TrashButton& trash = layout.trash;
	trash.enabled = true;
	trash.sheetImage = doc_.getString(kTrashImage);
	if (trash.sheetImage.empty()) {
		throw ConfigError(Kind::InvalidValue, kTrashImage);
	}
	const std::array<int, 2> cell = doc_.getIntArray<2>(kTrashCell);
	trash.cellWidth = cell[0];
	trash.cellHeight = cell[1];
	if (trash.cellWidth <= 0 || trash.cellHeight <= 0) {
		throw ConfigError(Kind::InvalidValue, kTrashCell);
	}
	trash.size = doc_.getDouble(kTrashSize);
	if (trash.size <= 0.0) {
		throw ConfigError(Kind::InvalidValue, kTrashSize);
	}
	trash.gap = doc_.getInt(kTrashGap);
	if (trash.gap < 0) {
		throw ConfigError(Kind::InvalidValue, kTrashGap);
	}
	trash.iconPad =
		doc_.hasKey(kTrashIconPad) ? doc_.getInt(kTrashIconPad) : 0;
	if (trash.iconPad < 0) {
		throw ConfigError(Kind::InvalidValue, kTrashIconPad);
	}
	if (doc_.hasKey(kTrashHoverTint)) {
		const std::array<int, 3> tint = doc_.getIntArray<3>(kTrashHoverTint);
		for (const int channel : tint) {
			if (channel < 0 || channel > 255) {
				throw ConfigError(Kind::InvalidValue, kTrashHoverTint);
			}
		}
		trash.hoverTint = Color{tint[0], tint[1], tint[2], 255};
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
	return SheetCell(doc_, key);
}

InventoryAnchor InventoryTomlReader::parseAnchor(
	const std::string& anchor) const {
	if (anchor == kAnchorBottomCenter) {
		return InventoryAnchor::BottomCenter;
	}
	throw ConfigError(Kind::InvalidValue, kBarAnchor);
}

}
