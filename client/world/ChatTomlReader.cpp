#include "world/ChatTomlReader.h"

#include <array>

#include "world/ConfigError.h"

namespace world {

namespace {

using Kind = ConfigErrorKind;

constexpr const char* kChat = "chat";
constexpr const char* kSheet = "sheet";
constexpr const char* kCell = "cell";
constexpr const char* kBorder = "border";
constexpr const char* kWidthFraction = "width_fraction";
constexpr const char* kMarginX = "margin_x";
constexpr const char* kMarginBottom = "margin_bottom";
constexpr const char* kInputFraction = "input_fraction";
constexpr const char* kTextPad = "text_pad";
constexpr const char* kFontSize = "font_size";
constexpr const char* kFont = "font";
constexpr const char* kColor = "color";
constexpr const char* kPreviewAlpha = "preview_alpha";
constexpr const char* kPreviewRows = "preview_rows";

}

ChatTomlReader::ChatTomlReader(const tomlcfg::TomlDocument& doc)
	: doc_(doc) {}

ChatLayout ChatTomlReader::read() const {
	if (!doc_.hasKey(kChat)) {
		throw ConfigError(Kind::MissingStructure, kChat);
	}

	const tomlcfg::TomlDocument chat = doc_.getSubtable(kChat);

	ChatLayout layout;
	layout.sheetImage = chat.getString(kSheet);
	if (layout.sheetImage.empty()) {
		throw ConfigError(Kind::InvalidValue, kSheet);
	}

	const std::array<int, 2> cell = chat.getIntArray<2>(kCell);
	layout.sheetCellWidth = cell[0];
	layout.sheetCellHeight = cell[1];
	if (layout.sheetCellWidth <= 0 || layout.sheetCellHeight <= 0) {
		throw ConfigError(Kind::InvalidValue, kCell);
	}

	// El grosor del marco puede ser 0 (sin 9-slice), pero no negativo.
	layout.sheetBorder = chat.getInt(kBorder);
	if (layout.sheetBorder < 0) {
		throw ConfigError(Kind::InvalidValue, kBorder);
	}

	layout.widthFraction = chat.getDouble(kWidthFraction);
	if (layout.widthFraction <= 0.0 || layout.widthFraction > 1.0) {
		throw ConfigError(Kind::InvalidValue, kWidthFraction);
	}

	layout.marginX = chat.getInt(kMarginX);
	layout.marginBottom = chat.getInt(kMarginBottom);

	layout.inputFraction = chat.getDouble(kInputFraction);
	if (layout.inputFraction <= 0.0 || layout.inputFraction > 1.0) {
		throw ConfigError(Kind::InvalidValue, kInputFraction);
	}

	layout.textPad = chat.getInt(kTextPad);
	layout.fontSize = chat.getInt(kFontSize);
	// Cara de fuente opcional (ui/text.toml [[font.face]]).
	if (chat.hasKey(kFont)) {
		layout.fontFace = chat.getString(kFont);
	}
	if (layout.textPad < 0) {
		throw ConfigError(Kind::InvalidValue, kTextPad);
	}
	if (layout.fontSize <= 0) {
		throw ConfigError(Kind::InvalidValue, kFontSize);
	}

	const std::array<int, 4> color = chat.getIntArray<4>(kColor);
	layout.textColor = Color{color[0], color[1], color[2], color[3]};

	layout.previewAlpha = chat.getInt(kPreviewAlpha);
	if (layout.previewAlpha < 0 || layout.previewAlpha > 255) {
		throw ConfigError(Kind::InvalidValue, kPreviewAlpha);
	}

	layout.previewRows = chat.getInt(kPreviewRows);
	if (layout.previewRows <= 0) {
		throw ConfigError(Kind::InvalidValue, kPreviewRows);
	}

	return layout;
}

}
