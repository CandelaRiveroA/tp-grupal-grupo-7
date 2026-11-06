#include "world/PromptLayout.h"

#include <array>
#include <cstddef>
#include <utility>

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"

namespace world {

PromptLayout::PromptLayout(const std::string& path) {
	try {
		const tomlcfg::TomlDocument doc(path,
						tomlcfg::TomlDocument::Source::File);
		const tomlcfg::TomlDocument prompt = doc.getSubtable("prompt");
		question = prompt.getString("question");
		options = prompt.getStringArray("options");
		waiting = prompt.getString("waiting");
		const std::array<int, 4> text = prompt.getIntArray<4>("text_color");
		textColor = {text[0], text[1], text[2], text[3]};
		const std::array<int, 4> hover = prompt.getIntArray<4>("hover_color");
		hoverColor = {hover[0], hover[1], hover[2], hover[3]};
		textSize = prompt.getInt("text_size");
		lineGap = prompt.getInt("line_gap");
		const std::array<int, 2> gridData = prompt.getIntArray<2>("grid");
		grid = {gridData[0], gridData[1]};
		readCancel(doc);
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
	if (options.empty()) {
		throw ConfigError(ConfigErrorKind::InvalidValue, "prompt.options");
	}
	if (textSize <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue, "prompt.text_size");
	}
	if (lineGap < 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue, "prompt.line_gap");
	}
	if (grid.columns <= 0 || grid.rows <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue, "prompt.grid");
	}
}

PixelBox PromptLayout::questionBox(const PixelBox& panel) const {
	// El bloque: la pregunta y, debajo, la fila de opciones.
	const int lines = 2;
	const int totalHeight = lines * textSize + lineGap;
	const int startY = panel.y + (panel.height - totalHeight) / 2;
	return PixelBox{panel.x, startY, panel.width, textSize};
}

std::vector<PixelBox> PromptLayout::optionBoxes(const PixelBox& panel) const {
	// Una columna por opción, en la fila de abajo del bloque.
	std::vector<PixelBox> boxes;
	boxes.reserve(options.size());
	const PixelBox question = questionBox(panel);
	const int rowY = question.y + question.height + lineGap;
	const int columnWidth =
		panel.width / static_cast<int>(options.size());
	for (std::size_t i = 0; i < options.size(); ++i) {
		boxes.push_back(PixelBox{
			panel.x + static_cast<int>(i) * columnWidth, rowY,
			columnWidth, textSize});
	}
	return boxes;
}

PixelBox PromptLayout::waitingBox(const PixelBox& panel) const {
	return PixelBox{panel.x, panel.y + (panel.height - textSize) / 2,
			panel.width, textSize};
}

int PromptLayout::optionAt(const PixelBox& panel,
			   const PixelPoint& point) const {
	const std::vector<PixelBox> boxes = optionBoxes(panel);
	for (std::size_t i = 0; i < boxes.size(); ++i) {
		if (contains(boxes[i], point)) {
			return static_cast<int>(i);
		}
	}
	return -1;
}

PixelBox PromptLayout::cancelBox(const PixelBox& panel) const {
	// Abajo a la derecha del panel, con el margen de los datos.
	return PixelBox{panel.x + panel.width - cancel.margin.width -
				cancel.size.width,
			panel.y + panel.height - cancel.margin.height -
				cancel.size.height,
			cancel.size.width, cancel.size.height};
}

bool PromptLayout::cancelAt(const PixelBox& panel,
			    const PixelPoint& point) const {
	return contains(cancelBox(panel), point);
}

std::vector<FrameLayout::Piece> PromptLayout::cancelPieces(
	const PixelBox& button) const {
	// El 9-slice estirado a la caja del botón: esquinas de
	// border×border, bordes entre ellas y relleno al centro.
	const int border = cancel.border;
	const int innerW = button.width - 2 * border;
	const int innerH = button.height - 2 * border;
	const std::array<std::pair<const ButtonPiece*, PixelBox>, 9> pieces = {{
		{&cancel.topLeft, {button.x, button.y, border, border}},
		{&cancel.top,
		 {button.x + border, button.y, innerW, border}},
		{&cancel.topRight,
		 {button.x + button.width - border, button.y, border, border}},
		{&cancel.left,
		 {button.x, button.y + border, border, innerH}},
		{&cancel.fill, {button.x + border, button.y + border, innerW,
				innerH}},
		{&cancel.right, {button.x + button.width - border,
				 button.y + border, border, innerH}},
		{&cancel.bottomLeft,
		 {button.x, button.y + button.height - border, border,
		  border}},
		{&cancel.bottom,
		 {button.x + border, button.y + button.height - border,
		  innerW, border}},
		{&cancel.bottomRight,
		 {button.x + button.width - border,
		  button.y + button.height - border, border, border}},
	}};
	std::vector<FrameLayout::Piece> drawn;
	drawn.reserve(pieces.size());
	for (const std::pair<const ButtonPiece*, PixelBox>& piece : pieces) {
		drawn.push_back(FrameLayout::Piece{piece.first->cell,
						   piece.first->srcCols,
						   piece.first->srcRows,
						   piece.second, false});
	}
	return drawn;
}

ButtonPiece PromptLayout::readPiece(const tomlcfg::TomlDocument& doc,
				    const std::string& path) const {
	const std::array<int, 4> piece = doc.getIntArray<4>(path);
	if (piece[0] < 0 || piece[1] < 0 || piece[2] <= 0 || piece[3] <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue, path);
	}
	return ButtonPiece{SheetCell{piece[0], piece[1]}, piece[2], piece[3]};
}

void PromptLayout::readCancel(const tomlcfg::TomlDocument& doc) {
	cancel.sheet = doc.getString("prompt.cancel.sheet");
	const std::array<int, 2> cell =
		doc.getIntArray<2>("prompt.cancel.cell");
	cancel.cellWidth = cell[0];
	cancel.cellHeight = cell[1];
	cancel.topLeft = readPiece(doc, "prompt.cancel.top_left");
	cancel.top = readPiece(doc, "prompt.cancel.top");
	cancel.topRight = readPiece(doc, "prompt.cancel.top_right");
	cancel.left = readPiece(doc, "prompt.cancel.left");
	cancel.fill = readPiece(doc, "prompt.cancel.fill");
	cancel.right = readPiece(doc, "prompt.cancel.right");
	cancel.bottomLeft = readPiece(doc, "prompt.cancel.bottom_left");
	cancel.bottom = readPiece(doc, "prompt.cancel.bottom");
	cancel.bottomRight = readPiece(doc, "prompt.cancel.bottom_right");
	cancel.border = doc.getInt("prompt.cancel.border");
	const std::array<int, 2> size =
		doc.getIntArray<2>("prompt.cancel.size");
	cancel.size = {size[0], size[1]};
	const std::array<int, 2> margin =
		doc.getIntArray<2>("prompt.cancel.margin");
	cancel.margin = {margin[0], margin[1]};

	if (cancel.sheet.empty()) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  "prompt.cancel.sheet");
	}
	if (cancel.cellWidth <= 0 || cancel.cellHeight <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  "prompt.cancel.cell");
	}
	if (cancel.border <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  "prompt.cancel.border");
	}
	if (cancel.size.width <= 2 * cancel.border ||
	    cancel.size.height <= 2 * cancel.border) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  "prompt.cancel.size");
	}
	if (cancel.margin.width < 0 || cancel.margin.height < 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  "prompt.cancel.margin");
	}
}

bool PromptLayout::contains(const PixelBox& box,
			    const PixelPoint& point) const {
	return point.x >= box.x && point.x < box.x + box.width &&
	       point.y >= box.y && point.y < box.y + box.height;
}

}  // namespace world
