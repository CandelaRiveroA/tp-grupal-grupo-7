#include "world/ToolOverlayTomlReader.h"

#include <array>
#include <vector>

#include "world/ConfigError.h"

namespace world {

namespace {

using tomlcfg::TomlDocument;
using Kind = ConfigErrorKind;

// Claves del TOML.
constexpr const char* kOffset = "offset";
constexpr const char* kSheet = "sheet";
constexpr const char* kCell = "cell";
constexpr const char* kFrameTicks = "frame_ticks";
constexpr const char* kFrames = "frames";
constexpr const char* kPivot = "pivot";
constexpr const char* kLayers = "layers";
constexpr const char* kAnchor = "anchor";
constexpr const char* kOffsetX = "offset_x";
constexpr const char* kOffsetYFrame = "offset_y";
constexpr const char* kAngle = "angle";
constexpr const char* kFlip = "flip";
constexpr const char* kScale = "scale";

constexpr const char* kAnchorTop = "top";
constexpr const char* kAnchorBody = "body";
constexpr const char* kAnchorBottom = "bottom";

// Direcciones en el mismo orden que world::Direction:
// Front=0, Back=1, Left=2, Right=3.
constexpr std::array<const char*, 4> kDirectionKeys = {
	"front", "back", "left", "right"};

// Ítem lógico -> tabla del TOML que describe su overlay.
struct ItemBinding {
	ItemKind kind;
	const char* tableKey;
};

constexpr std::array<ItemBinding, 4> kItemBindings = {{
	{ItemKind::Pickaxe, "pickaxe"},
	{ItemKind::Shovel, "shovel"},
	{ItemKind::WateringCan, "watering_can"},
	{ItemKind::Sword, "sword"},
}};

}

ToolOverlayTomlReader::ToolOverlayTomlReader(const TomlDocument& doc)
	: doc_(doc) {}

std::string ToolOverlayTomlReader::join(const std::string& base,
					const std::string& key) const {
	return base + "." + key;
}

std::string ToolOverlayTomlReader::idx(const std::string& base,
				       std::size_t i) const {
	return base + "[" + std::to_string(i) + "]";
}

std::map<std::string, FrameAnchor> ToolOverlayTomlReader::anchorNames() const {
	return {
		{kAnchorTop, FrameAnchor::Top},
		{kAnchorBody, FrameAnchor::Body},
		{kAnchorBottom, FrameAnchor::Bottom},
	};
}

LayerFrame ToolOverlayTomlReader::readLayer(
	const TomlDocument& layerDoc, const std::string& layerPath) const {
	const std::size_t n = layerDoc.arraySize(layerPath);
	if (n == 2) {
		const std::array<int, 2> a = layerDoc.getIntArray<2>(layerPath);
		if (a[0] < 0 || a[1] < 0) {
			throw ConfigError(Kind::InvalidValue, layerPath);
		}
		return LayerFrame{Layer::Body, SheetCell{a[0], a[1]}, 1, 1};
	}
	if (n == 4) {
		const std::array<int, 4> a = layerDoc.getIntArray<4>(layerPath);
		if (a[0] < 0 || a[1] < 0 || a[2] <= 0 || a[3] <= 0) {
			throw ConfigError(Kind::InvalidValue, layerPath);
		}
		return LayerFrame{Layer::Body, SheetCell{a[0], a[1]}, a[2],
				  a[3]};
	}
	throw ConfigError(Kind::InvalidValue, layerPath);
}

FrameAnchor ToolOverlayTomlReader::readAnchor(
	const TomlDocument& frameDoc) const {
	if (!frameDoc.hasKey(kAnchor)) {
		return FrameAnchor::Top;
	}
	const std::string value = frameDoc.getString(kAnchor);
	const std::map<std::string, FrameAnchor> names = anchorNames();
	const auto it = names.find(value);
	if (it == names.end()) {
		throw ConfigError(Kind::InvalidValue, kAnchor);
	}
	return it->second;
}

ToolOverlayFrame ToolOverlayTomlReader::readFrame(
	const TomlDocument& frameDoc) const {
	ToolOverlayFrame frame;
	frame.anchor = readAnchor(frameDoc);
	if (frameDoc.hasKey(kOffsetX)) {
		frame.offsetX = frameDoc.getInt(kOffsetX);
	}
	if (frameDoc.hasKey(kOffsetYFrame)) {
		frame.offsetY = frameDoc.getInt(kOffsetYFrame);
	}
	if (frameDoc.hasKey(kAngle)) {
		frame.angle = frameDoc.getDouble(kAngle);
	}
	if (frameDoc.hasKey(kPivot)) {
		frame.pivotX = frameDoc.getDouble(idx(kPivot, 0));
		frame.pivotY = frameDoc.getDouble(idx(kPivot, 1));
	}
	if (frameDoc.hasKey(kFlip)) {
		frame.flip = frameDoc.getBool(kFlip);
	}
	if (frameDoc.hasKey(kScale)) {
		frame.scale = frameDoc.getDouble(kScale);
	}

	const std::size_t layers = frameDoc.arraySize(kLayers);
	for (std::size_t i = 0; i < layers; ++i) {
		frame.layers.push_back(readLayer(frameDoc, idx(kLayers, i)));
	}
	return frame;
}

ToolOverlayAction ToolOverlayTomlReader::readAction(
	const std::string& base) const {
	ToolOverlayAction action;

	const std::string sheetPath = join(base, kSheet);
	action.sheetImage = doc_.getString(sheetPath);
	if (action.sheetImage.empty()) {
		throw ConfigError(Kind::InvalidValue, sheetPath);
	}

	const std::string cellPath = join(base, kCell);
	const std::array<int, 2> cell = doc_.getIntArray<2>(cellPath);
	if (cell[0] <= 0 || cell[1] <= 0) {
		throw ConfigError(Kind::InvalidValue, cellPath);
	}
	action.cellWidth = cell[0];
	action.cellHeight = cell[1];

	const std::string ticksPath = join(base, kFrameTicks);
	action.frameTicks = doc_.getInt(ticksPath);
	if (action.frameTicks <= 0) {
		throw ConfigError(Kind::InvalidValue, ticksPath);
	}

	// getTableArray preserva el orden de los frames (cada frame es una
	// tabla inline); recorrerlos con paths [i] los desordenaba.
	const std::string framesPath = join(base, kFrames);
	const std::vector<TomlDocument> frames =
		doc_.getTableArray(framesPath);
	if (frames.empty()) {
		throw ConfigError(Kind::InvalidValue, framesPath);
	}
	for (const TomlDocument& frameDoc : frames) {
		action.frames.push_back(readFrame(frameDoc));
	}
	return action;
}

ToolOverlayAppearance ToolOverlayTomlReader::readAppearance(
	const std::string& itemKey) const {
	ToolOverlayAppearance appearance;
	for (std::size_t i = 0; i < kDirectionKeys.size(); ++i) {
		const std::string dirBase = join(itemKey, kDirectionKeys[i]);
		if (!doc_.hasKey(dirBase)) {
			continue;
		}
		const Direction dir = static_cast<Direction>(i);
		appearance.emplace(dir, readAction(dirBase));
	}
	return appearance;
}

ToolOverlayCatalog ToolOverlayTomlReader::read() const {
	ToolOverlayCatalog catalog;
	catalog.anchorOffsets = readAnchorOffsets();
	for (const ItemBinding& binding : kItemBindings) {
		if (!doc_.hasKey(binding.tableKey)) {
			continue;
		}
		catalog.tools.emplace(binding.kind,
				      readAppearance(binding.tableKey));
	}
	return catalog;
}

std::map<FrameAnchor, int> ToolOverlayTomlReader::readAnchorOffsets() const {
	std::map<FrameAnchor, int> offsets;
	if (!doc_.hasKey(kOffset)) {
		return offsets;
	}
	// La tabla [offset] mapea nombre de anclaje -> px. Reusa el mismo
	// vocabulario de anclajes que los frames (anchorNames).
	for (const auto& [name, anchor] : anchorNames()) {
		const std::string path = join(kOffset, name);
		if (doc_.hasKey(path)) {
			offsets.emplace(anchor, doc_.getInt(path));
		}
	}
	return offsets;
}

}
