#include "world/PromptLayout.h"

#include <array>
#include <cstddef>
#include <optional>
#include <utility>

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"
#include "world/WorldConstants.h"

namespace world {

PromptLayout::PromptLayout(const std::string& path) {
	try {
		const tomlcfg::TomlDocument doc(path,
						tomlcfg::TomlDocument::Source::File);
		const tomlcfg::TomlDocument prompt = doc.getSubtable("prompt");
		question = prompt.getString("question");
		options = prompt.getStringArray("options");
		waiting = prompt.getString("waiting");
		nextDay = prompt.getString("next_day");
		const std::array<int, 4> text = prompt.getIntArray<4>("text_color");
		textColor = {text[0], text[1], text[2], text[3]};
		const std::array<int, 4> hover = prompt.getIntArray<4>("hover_color");
		hoverColor = {hover[0], hover[1], hover[2], hover[3]};
		textSize = prompt.getInt("text_size");
		lineGap = prompt.getInt("line_gap");
		const std::array<int, 2> gridData = prompt.getIntArray<2>("grid");
		grid = {gridData[0], gridData[1]};
		readButton(doc, kPromptCancelButton);
		readButton(doc, kPromptOkButton);
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

PixelBox PromptLayout::nextDayBox(const PixelBox& panel) const {
	// Centrada en la mitad de arriba del panel, sobre el botón.
	return PixelBox{panel.x, panel.y + (panel.height / 2 - textSize) / 2,
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

PixelBox PromptLayout::buttonBox(const std::string& name,
				 const PixelBox& panel) const {
	// Según el anchor y el margen de los datos del botón, dentro del
	// panel (AnchorLayout es la cuenta única de "alinear contra un borde").
	const ButtonFrame& button = buttons.at(name);
	PixelBox box = AnchorLayout().box(
		button.anchor, button.margin.width, button.margin.height,
		button.size.width, button.size.height, panel.width,
		panel.height);
	box.x += panel.x;
	box.y += panel.y;
	return box;
}

bool PromptLayout::buttonAt(const std::string& name, const PixelBox& panel,
			    const PixelPoint& point) const {
	return contains(buttonBox(name, panel), point);
}

std::vector<FrameLayout::Piece> PromptLayout::buttonPieces(
	const std::string& name, const PixelBox& button) const {
	// El 9-slice estirado a la caja del botón: esquinas de
	// border×border, bordes entre ellas y relleno al centro.
	const ButtonFrame& frame = buttons.at(name);
	const int border = frame.border;
	const int innerW = button.width - 2 * border;
	const int innerH = button.height - 2 * border;
	const std::array<std::pair<const ButtonPiece*, PixelBox>, 9> pieces = {{
		{&frame.topLeft, {button.x, button.y, border, border}},
		{&frame.top,
		 {button.x + border, button.y, innerW, border}},
		{&frame.topRight,
		 {button.x + button.width - border, button.y, border, border}},
		{&frame.left,
		 {button.x, button.y + border, border, innerH}},
		{&frame.fill, {button.x + border, button.y + border, innerW,
			       innerH}},
		{&frame.right, {button.x + button.width - border,
				button.y + border, border, innerH}},
		{&frame.bottomLeft,
		 {button.x, button.y + button.height - border, border,
		  border}},
		{&frame.bottom,
		 {button.x + border, button.y + button.height - border,
		  innerW, border}},
		{&frame.bottomRight,
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

void PromptLayout::readButton(const tomlcfg::TomlDocument& doc,
			      const std::string& name) {
	const std::string base = "prompt." + name + ".";
	ButtonFrame frame;
	frame.sheet = doc.getString(base + "sheet");
	const std::array<int, 2> cell = doc.getIntArray<2>(base + "cell");
	frame.cellWidth = cell[0];
	frame.cellHeight = cell[1];
	frame.topLeft = readPiece(doc, base + "top_left");
	frame.top = readPiece(doc, base + "top");
	frame.topRight = readPiece(doc, base + "top_right");
	frame.left = readPiece(doc, base + "left");
	frame.fill = readPiece(doc, base + "fill");
	frame.right = readPiece(doc, base + "right");
	frame.bottomLeft = readPiece(doc, base + "bottom_left");
	frame.bottom = readPiece(doc, base + "bottom");
	frame.bottomRight = readPiece(doc, base + "bottom_right");
	frame.border = doc.getInt(base + "border");
	const std::array<int, 2> size = doc.getIntArray<2>(base + "size");
	frame.size = {size[0], size[1]};
	const std::array<int, 2> margin = doc.getIntArray<2>(base + "margin");
	frame.margin = {margin[0], margin[1]};
	// Sin `anchor`, el botón va abajo a la derecha (el caso del cancelar).
	if (doc.hasKey(base + "anchor")) {
		const std::optional<Anchor> anchor =
			AnchorLayout().fromName(doc.getString(base + "anchor"));
		if (!anchor.has_value()) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  base + "anchor");
		}
		frame.anchor = *anchor;
	}

	if (frame.sheet.empty()) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  base + "sheet");
	}
	if (frame.cellWidth <= 0 || frame.cellHeight <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  base + "cell");
	}
	if (frame.border <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  base + "border");
	}
	if (frame.size.width <= 2 * frame.border ||
	    frame.size.height <= 2 * frame.border) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  base + "size");
	}
	if (frame.margin.width < 0 || frame.margin.height < 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  base + "margin");
	}
	buttons.emplace(name, frame);
}

bool PromptLayout::contains(const PixelBox& box,
			    const PixelPoint& point) const {
	return point.x >= box.x && point.x < box.x + box.width &&
	       point.y >= box.y && point.y < box.y + box.height;
}

}  // namespace world
