#include "world/HudOverlayTomlReader.h"

#include <array>
#include <utility>

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"

namespace world {

namespace {

using Kind = ConfigErrorKind;

constexpr const char* kOverlays = "overlay";
constexpr const char* kLabels = "label";
constexpr const char* kText = "text";
constexpr const char* kBox = "box";
constexpr const char* kFontSize = "font_size";
constexpr const char* kLabelColor = "color";
constexpr const char* kTokens = "token";
constexpr const char* kSource = "source";
constexpr const char* kDivisor = "divisor";
constexpr const char* kModulo = "modulo";
constexpr const char* kPad = "pad";
constexpr const char* kNames = "names";
constexpr const char* kSheetImage = "sheet";
constexpr const char* kCell = "cell";
constexpr const char* kRegion = "region";
constexpr const char* kAnchor = "anchor";
constexpr const char* kMargin = "margin";
constexpr const char* kScale = "scale";
constexpr const char* kAlpha = "alpha";
constexpr const char* kColor = "color";
constexpr const char* kBlend = "blend";
constexpr const char* kBlendAlpha = "alpha";
constexpr const char* kBlendMultiply = "multiply";
constexpr const char* kFill = "fill";
constexpr const char* kValue = "value";
constexpr const char* kCavity = "cavity";

constexpr int kDefaultScale = 1;
constexpr int kMaxAlpha = 255;

}  // namespace

HudOverlayTomlReader::HudOverlayTomlReader(const tomlcfg::TomlDocument& doc)
	: doc_(doc) {}

HudOverlayCatalog HudOverlayTomlReader::read() const {
	HudOverlayCatalog catalog;
	if (doc_.hasKey(kOverlays)) {
		for (const tomlcfg::TomlDocument& overlayDoc :
		     doc_.getTableArray(kOverlays)) {
			catalog.add(readOverlay(overlayDoc));
		}
	}
	if (doc_.hasKey(kLabels)) {
		for (const tomlcfg::TomlDocument& labelDoc :
		     doc_.getTableArray(kLabels)) {
			catalog.addLabel(readLabel(labelDoc));
		}
	}
	return catalog;
}

HudLabel HudOverlayTomlReader::readLabel(
	const tomlcfg::TomlDocument& labelDoc) const {
	HudLabel label;

	label.text = labelDoc.getString(kText);
	if (label.text.empty()) {
		throw ConfigError(Kind::InvalidValue, kText);
	}

	label.anchor = readAnchor(labelDoc);

	if (labelDoc.hasKey(kMargin)) {
		const std::array<int, 2> margin =
			labelDoc.getIntArray<2>(kMargin);
		label.marginX = margin[0];
		label.marginY = margin[1];
	}

	const std::array<int, 2> box = labelDoc.getIntArray<2>(kBox);
	label.boxWidth = box[0];
	label.boxHeight = box[1];
	if (label.boxWidth <= 0 || label.boxHeight <= 0) {
		throw ConfigError(Kind::InvalidValue, kBox);
	}

	label.fontSize = labelDoc.getInt(kFontSize);
	if (label.fontSize <= 0) {
		throw ConfigError(Kind::InvalidValue, kFontSize);
	}

	const std::array<int, 4> color = labelDoc.getIntArray<4>(kLabelColor);
	for (const int channel : color) {
		if (channel < 0 || channel > kMaxAlpha) {
			throw ConfigError(Kind::InvalidValue, kLabelColor);
		}
	}
	label.color = Color{color[0], color[1], color[2], color[3]};

	if (labelDoc.hasKey(kTokens)) {
		const tomlcfg::TomlDocument tokens = labelDoc.getSubtable(kTokens);
		for (const std::string& name : labelDoc.subtableKeys(kTokens)) {
			label.tokens[name] = readToken(tokens.getSubtable(name));
		}
	}
	return label;
}

HudTokenFormat HudOverlayTomlReader::readToken(
	const tomlcfg::TomlDocument& tokenDoc) const {
	HudTokenFormat format;
	if (tokenDoc.hasKey(kSource)) {
		format.source = tokenDoc.getString(kSource);
	}
	if (tokenDoc.hasKey(kDivisor)) {
		format.divisor = tokenDoc.getInt(kDivisor);
		if (format.divisor <= 0) {
			throw ConfigError(Kind::InvalidValue, kDivisor);
		}
	}
	if (tokenDoc.hasKey(kModulo)) {
		format.modulo = tokenDoc.getInt(kModulo);
		if (format.modulo < 0) {
			throw ConfigError(Kind::InvalidValue, kModulo);
		}
	}
	if (tokenDoc.hasKey(kPad)) {
		format.pad = tokenDoc.getInt(kPad);
		if (format.pad < 0) {
			throw ConfigError(Kind::InvalidValue, kPad);
		}
	}
	if (tokenDoc.hasKey(kNames)) {
		format.names = tokenDoc.getStringArray(kNames);
	}
	return format;
}

HudOverlay HudOverlayTomlReader::readOverlay(
	const tomlcfg::TomlDocument& overlayDoc) const {
	HudOverlay overlay;

	if (overlayDoc.hasKey(kColor)) {
		// Velo de color plano: no lleva hoja. Mezclar ambos es ambiguo.
		if (overlayDoc.hasKey(kSheetImage) || overlayDoc.hasKey(kFill)) {
			throw ConfigError(Kind::InvalidValue, kColor);
		}
		overlay.color = readColor(overlayDoc);
		overlay.blend = readBlend(overlayDoc);
		readAlpha(overlayDoc, overlay);
		return overlay;
	}

	overlay.sheetImage = overlayDoc.getString(kSheetImage);
	if (overlay.sheetImage.empty()) {
		throw ConfigError(Kind::InvalidValue, kSheetImage);
	}

	const std::array<int, 2> cell = overlayDoc.getIntArray<2>(kCell);
	overlay.cellWidth = cell[0];
	overlay.cellHeight = cell[1];
	if (overlay.cellWidth <= 0 || overlay.cellHeight <= 0) {
		throw ConfigError(Kind::InvalidValue, kCell);
	}

	overlay.region = readRegion(overlayDoc);
	overlay.anchor = readAnchor(overlayDoc);

	if (overlayDoc.hasKey(kMargin)) {
		const std::array<int, 2> margin =
			overlayDoc.getIntArray<2>(kMargin);
		overlay.marginX = margin[0];
		overlay.marginY = margin[1];
	}

	overlay.scale = overlayDoc.hasKey(kScale) ? overlayDoc.getInt(kScale)
						  : kDefaultScale;
	if (overlay.scale <= 0) {
		throw ConfigError(Kind::InvalidValue, kScale);
	}

	readAlpha(overlayDoc, overlay);

	if (overlayDoc.hasKey(kFill)) {
		overlay.fill = readFill(overlayDoc.getSubtable(kFill), overlay);
	}

	return overlay;
}

HudFill HudOverlayTomlReader::readFill(
	const tomlcfg::TomlDocument& fillDoc,
	const HudOverlay& overlay) const {
	HudFill fill;

	fill.value = fillDoc.getString(kValue);
	if (fill.value.empty()) {
		throw ConfigError(Kind::InvalidValue, kValue);
	}

	fill.sheetImage = fillDoc.getString(kSheetImage);
	if (fill.sheetImage.empty()) {
		throw ConfigError(Kind::InvalidValue, kSheetImage);
	}

	const std::array<int, 2> cell = fillDoc.getIntArray<2>(kCell);
	fill.cellWidth = cell[0];
	fill.cellHeight = cell[1];
	if (fill.cellWidth <= 0 || fill.cellHeight <= 0) {
		throw ConfigError(Kind::InvalidValue, kCell);
	}

	fill.region = readRegion(fillDoc);
	if (fill.region.cols != 1 || fill.region.rows != 1) {
		// El relleno se estira desde UNA celda: el contenido opaco de
		// esa celda es el que llena la cavidad.
		throw ConfigError(Kind::InvalidValue, kRegion);
	}

	const std::array<int, 4> cavity = fillDoc.getIntArray<4>(kCavity);
	fill.cavityX = cavity[0];
	fill.cavityY = cavity[1];
	fill.cavityWidth = cavity[2];
	fill.cavityHeight = cavity[3];
	if (fill.cavityX < 0 || fill.cavityY < 0 ||
	    fill.cavityWidth <= 0 || fill.cavityHeight <= 0) {
		throw ConfigError(Kind::InvalidValue, kCavity);
	}
	// La cavidad es relativa a la región del overlay (la cáscara).
	const int shellWidth = overlay.region.cols * overlay.cellWidth;
	const int shellHeight = overlay.region.rows * overlay.cellHeight;
	if (fill.cavityX + fill.cavityWidth > shellWidth ||
	    fill.cavityY + fill.cavityHeight > shellHeight) {
		throw ConfigError(Kind::InvalidValue, kCavity);
	}

	return fill;
}

void HudOverlayTomlReader::readAlpha(const tomlcfg::TomlDocument& overlayDoc,
				     HudOverlay& overlay) const {
	if (overlayDoc.hasKey(kAlpha)) {
		overlay.alpha = overlayDoc.getInt(kAlpha);
		if (overlay.alpha < 0 || overlay.alpha > kMaxAlpha) {
			throw ConfigError(Kind::InvalidValue, kAlpha);
		}
	}
}

Color HudOverlayTomlReader::readColor(
	const tomlcfg::TomlDocument& overlayDoc) const {
	// color = [r, g, b], cada canal 0..255. El alpha va en `alpha`.
	const std::array<int, 3> rgb = overlayDoc.getIntArray<3>(kColor);
	for (const int channel : rgb) {
		if (channel < 0 || channel > kMaxAlpha) {
			throw ConfigError(Kind::InvalidValue, kColor);
		}
	}
	return Color{rgb[0], rgb[1], rgb[2], kMaxAlpha};
}

SheetRegion HudOverlayTomlReader::readRegion(
	const tomlcfg::TomlDocument& overlayDoc) const {
	// region = [col, row, cols, rows]: celda de origen y cuántas celdas
	// abarca. Sin la clave, la región es una sola celda en (0, 0).
	if (!overlayDoc.hasKey(kRegion)) {
		return SheetRegion(SheetCell(0, 0), 1, 1);
	}
	const std::array<int, 4> region = overlayDoc.getIntArray<4>(kRegion);
	const int cols = region[2];
	const int rows = region[3];
	if (region[0] < 0 || region[1] < 0 || cols <= 0 || rows <= 0) {
		throw ConfigError(Kind::InvalidValue, kRegion);
	}
	return SheetRegion(SheetCell(region[0], region[1]), cols, rows);
}

Anchor HudOverlayTomlReader::readAnchor(
	const tomlcfg::TomlDocument& overlayDoc) const {
	// Sin clave, el default es top_right (el caso del HUD de tiempo).
	if (!overlayDoc.hasKey(kAnchor)) {
		return Anchor::TopRight;
	}
	const std::optional<Anchor> anchor =
		AnchorLayout().fromName(overlayDoc.getString(kAnchor));
	if (!anchor.has_value()) {
		throw ConfigError(Kind::InvalidValue, kAnchor);
	}
	return *anchor;
}

std::map<std::string, BlendMode> HudOverlayTomlReader::blendNames() const {
	return {
		{kBlendAlpha, BlendMode::Alpha},
		{kBlendMultiply, BlendMode::Multiply},
	};
}

BlendMode HudOverlayTomlReader::readBlend(
	const tomlcfg::TomlDocument& overlayDoc) const {
	// Sin clave, el velo se mezcla con alpha (el caso por defecto).
	if (!overlayDoc.hasKey(kBlend)) {
		return BlendMode::Alpha;
	}
	const std::map<std::string, BlendMode> names = blendNames();
	const auto it = names.find(overlayDoc.getString(kBlend));
	if (it == names.end()) {
		throw ConfigError(Kind::InvalidValue, kBlend);
	}
	return it->second;
}

}
