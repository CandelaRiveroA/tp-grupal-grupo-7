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
constexpr const char* kAction = "action";
constexpr const char* kFrames = "frames";
constexpr const char* kPivot = "pivot";
constexpr const char* kLayers = "layers";
constexpr const char* kAnchor = "anchor";
constexpr const char* kOffsetX = "offset_x";
constexpr const char* kOffsetYFrame = "offset_y";
constexpr const char* kAngle = "angle";
constexpr const char* kFlip = "flip";
constexpr const char* kScale = "scale";
constexpr const char* kCellSpan = "cell_span";
constexpr const char* kDrawn = "drawn";
constexpr const char* kHeld = "held";
constexpr const char* kEffect = "effect";

constexpr const char* kAnchorTop = "top";
constexpr const char* kAnchorBody = "body";
constexpr const char* kAnchorBottom = "bottom";

// Direcciones en el mismo orden que world::Direction:
// Front=0, Back=1, Left=2, Right=3.
constexpr std::array<const char*, 4> kDirectionKeys = {
	"front", "back", "left", "right"};

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

OverlayLayer ToolOverlayTomlReader::readLayer(
	const TomlDocument& layerDoc, const std::string& layerPath) const {
	const std::size_t n = layerDoc.arraySize(layerPath);
	if (n == 2) {
		const std::array<int, 2> a = layerDoc.getIntArray<2>(layerPath);
		if (a[0] < 0 || a[1] < 0) {
			throw ConfigError(Kind::InvalidValue, layerPath);
		}
		return OverlayLayer{
			LayerFrame{Layer::Body, SheetCell{a[0], a[1]}, 1, 1},
			0, 0};
	}
	if (n == 4) {
		const std::array<int, 4> a = layerDoc.getIntArray<4>(layerPath);
		if (a[0] < 0 || a[1] < 0 || a[2] <= 0 || a[3] <= 0) {
			throw ConfigError(Kind::InvalidValue, layerPath);
		}
		return OverlayLayer{
			LayerFrame{Layer::Body, SheetCell{a[0], a[1]}, a[2],
				   a[3]},
			0, 0};
	}
	// 6 elementos: celda + offset EN CELDAS (dx, dy) respecto del ancla
	// del frame. Es lo que permite distribuir varias celdas en 2D (el
	// chorro de la regadera). El offset puede ser negativo (arriba/izq).
	if (n == 6) {
		const std::array<int, 6> a = layerDoc.getIntArray<6>(layerPath);
		if (a[0] < 0 || a[1] < 0 || a[2] <= 0 || a[3] <= 0) {
			throw ConfigError(Kind::InvalidValue, layerPath);
		}
		return OverlayLayer{
			LayerFrame{Layer::Body, SheetCell{a[0], a[1]}, a[2],
				   a[3]},
			a[4], a[5]};
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

void ToolOverlayTomlReader::readPlacement(
	const TomlDocument& frameDoc, OverlayFrame& frame) const {
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
	// cell_span = [cols, rows]: caja del patrón en celdas, para que todo
	// el frame entre en UNA celda lógica (el chorro no crece con el
	// grupo). Opcional; sin la clave, 0 (sin normalizar).
	if (frameDoc.hasKey(kCellSpan)) {
		const std::array<int, 2> span =
			frameDoc.getIntArray<2>(kCellSpan);
		if (span[0] <= 0 || span[1] <= 0) {
			throw ConfigError(Kind::InvalidValue, kCellSpan);
		}
		frame.cellSpanX = span[0];
		frame.cellSpanY = span[1];
	}
}

ToolOverlayFrame ToolOverlayTomlReader::readFrame(
	const TomlDocument& frameDoc) const {
	ToolOverlayFrame frame;
	readPlacement(frameDoc, frame);

	const std::size_t layers = frameDoc.arraySize(kLayers);
	for (std::size_t i = 0; i < layers; ++i) {
		frame.layers.push_back(readLayer(frameDoc, idx(kLayers, i)));
	}
	return frame;
}

HeldOverlayFrame ToolOverlayTomlReader::readHeldFrame(
	const TomlDocument& frameDoc) const {
	HeldOverlayFrame frame;
	readPlacement(frameDoc, frame);
	if (frameDoc.hasKey(kDrawn)) {
		frame.drawn = frameDoc.getBool(kDrawn);
	}
	return frame;
}

HeldContentOverlay ToolOverlayTomlReader::readHeldContent(
	const std::string& base) const {
	HeldContentOverlay overlay;

	const std::string actionPath = join(base, kAction);
	if (doc_.hasKey(actionPath)) {
		const ActionKind* kind =
			actionNames_.find(doc_.getString(actionPath));
		if (kind == nullptr) {
			throw ConfigError(Kind::InvalidValue, actionPath);
		}
		overlay.action = *kind;
	}

	const std::string ticksPath = join(base, kFrameTicks);
	overlay.frameTicks = doc_.getInt(ticksPath);
	if (overlay.frameTicks <= 0) {
		throw ConfigError(Kind::InvalidValue, ticksPath);
	}

	const std::string framesPath = join(base, kFrames);
	const std::vector<TomlDocument> frames =
		doc_.getTableArray(framesPath);
	if (frames.empty()) {
		throw ConfigError(Kind::InvalidValue, framesPath);
	}
	for (const TomlDocument& frameDoc : frames) {
		overlay.frames.push_back(readHeldFrame(frameDoc));
	}
	return overlay;
}

void ToolOverlayTomlReader::readAnimated(const std::string& base,
					 AnimatedOverlay& overlay) const {
	// Acción con la que se sincroniza el overlay (opcional; sin la
	// clave, use_tool: el caso histórico de la herramienta).
	const std::string actionPath = join(base, kAction);
	if (doc_.hasKey(actionPath)) {
		const ActionKind* kind =
			actionNames_.find(doc_.getString(actionPath));
		if (kind == nullptr) {
			throw ConfigError(Kind::InvalidValue, actionPath);
		}
		overlay.action = *kind;
	}

	const std::string sheetPath = join(base, kSheet);
	overlay.sheetImage = doc_.getString(sheetPath);
	if (overlay.sheetImage.empty()) {
		throw ConfigError(Kind::InvalidValue, sheetPath);
	}

	const std::string cellPath = join(base, kCell);
	const std::array<int, 2> cell = doc_.getIntArray<2>(cellPath);
	if (cell[0] <= 0 || cell[1] <= 0) {
		throw ConfigError(Kind::InvalidValue, cellPath);
	}
	overlay.cellWidth = cell[0];
	overlay.cellHeight = cell[1];

	const std::string ticksPath = join(base, kFrameTicks);
	overlay.frameTicks = doc_.getInt(ticksPath);
	if (overlay.frameTicks <= 0) {
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
		overlay.frames.push_back(readFrame(frameDoc));
	}
}

ToolOverlayAction ToolOverlayTomlReader::readAction(
	const std::string& base) const {
	ToolOverlayAction action;
	readAnimated(base, action);

	// Efecto opcional que la herramienta larga durante su acción (el
	// chorro de la regadera): otra animación con su propia hoja. Es el
	// MISMO dato de animación (readAnimated), así sumar un efecto a una
	// herramienta nueva es solo datos, sin tocar el dibujo.
	const std::string effectPath = join(base, kEffect);
	if (doc_.hasKey(effectPath)) {
		AnimatedOverlay effect;
		readAnimated(effectPath, effect);
		action.effect = effect;
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
	// Las tablas del TOML se llaman como los contentIds (el vocabulario
	// compartido): recorrerlas es recorrer el vocabulario.
	for (const auto& [kind, tableKey] : contentIds_.itemEntries()) {
		if (!doc_.hasKey(tableKey)) {
			continue;
		}
		catalog.tools.emplace(kind, readAppearance(tableKey));
	}
	// El contenido sostenido en las manos (la cosecha al recoger) es
	// UN overlay común por dirección, no uno por contenido.
	for (std::size_t i = 0; i < kDirectionKeys.size(); ++i) {
		const std::string dirBase = join(kHeld, kDirectionKeys[i]);
		if (!doc_.hasKey(dirBase)) {
			continue;
		}
		const Direction dir = static_cast<Direction>(i);
		catalog.heldContent.emplace(dir, readHeldContent(dirBase));
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
