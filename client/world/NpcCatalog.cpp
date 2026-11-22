#include "world/NpcCatalog.h"

#include <algorithm>
#include <array>
#include <string>
#include <utility>

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"

namespace world {

namespace {

constexpr const char* kDefaultStyle = "default";

}

bool NpcDef::hasPopup() const {
	return !text.empty();
}

GridCell NpcDef::cellIn(const NpcCells& placed) const {
	const auto found = placed.find(id);
	return found != placed.end() ? found->second : body.at;
}

NpcCatalog::NpcCatalog(const std::string& path) {
	try {
		const tomlcfg::TomlDocument doc(
			path, tomlcfg::TomlDocument::Source::File);
		readStyles(doc);
		const std::size_t count = doc.arraySize("npcs");
		std::map<std::string, bool> ids;
		for (std::size_t i = 0; i < count; ++i) {
			NpcDef npc = readNpc(doc, i);
			if (!ids.emplace(npc.id, true).second) {
				throw ConfigError(
					ConfigErrorKind::InvalidValue,
					"npcs[" + std::to_string(i) + "].id");
			}
			npcs_.push_back(std::move(npc));
		}
	} catch (const tomlcfg::TomlError& error) {
		throw ConfigError(error);
	}
}

void NpcCatalog::readStyles(const tomlcfg::TomlDocument& doc) {
	const std::vector<std::string> names =
		doc.subtableKeys("popup_styles");
	if (names.empty()) {
		throw ConfigError(ConfigErrorKind::MissingStructure,
				  "popup_styles");
	}
	for (const std::string& name : names) {
		styles_.emplace(name, readStyle(doc, name));
	}
	if (styles_.find(kDefaultStyle) == styles_.end()) {
		throw ConfigError(ConfigErrorKind::MissingStructure,
				  "popup_styles.default");
	}
}

NpcPopupStyle NpcCatalog::readStyle(const tomlcfg::TomlDocument& doc,
				    const std::string& name) const {
	const std::string base = "popup_styles." + name;
	const std::array<int, 2> grid =
		doc.getIntArray<2>(base + ".grid");
	const std::array<int, 4> portrait =
		doc.getIntArray<4>(base + ".portrait");
	const std::array<int, 4> text =
		doc.getIntArray<4>(base + ".text");
	const std::array<int, 4> color =
		doc.getIntArray<4>(base + ".text_color");

	NpcPopupStyle style;
	style.grid = GridSpan{grid[0], grid[1]};
	style.portrait = NpcGridBox{GridCell{portrait[0], portrait[1]},
				    GridSpan{portrait[2], portrait[3]}};
	style.text = NpcGridBox{GridCell{text[0], text[1]},
				GridSpan{text[2], text[3]}};
	style.portraitInset = doc.getInt(base + ".portrait_inset");
	if (doc.hasKey(base + ".portrait_offset")) {
		const std::array<int, 2> offset =
			doc.getIntArray<2>(base + ".portrait_offset");
		style.portraitOffset = PixelPoint{offset[0], offset[1]};
	}
	if (doc.hasKey(base + ".portrait_scale")) {
		style.portraitScale = doc.getDouble(base + ".portrait_scale");
	}
	style.textInset = doc.getInt(base + ".text_inset");
	if (doc.hasKey(base + ".text_offset")) {
		const std::array<int, 2> offset =
			doc.getIntArray<2>(base + ".text_offset");
		style.textOffset = PixelPoint{offset[0], offset[1]};
	}
	style.textSize = doc.getInt(base + ".text_size");
	style.lineGap = doc.getInt(base + ".line_gap");
	style.textColor = Color{color[0], color[1], color[2], color[3]};

	const auto fitsGrid = [](const GridSpan& size, const NpcGridBox& box) {
		return box.at.column >= 0 && box.at.row >= 0 &&
		       box.span.columns > 0 && box.span.rows > 0 &&
		       box.at.column + box.span.columns <= size.columns &&
		       box.at.row + box.span.rows <= size.rows;
	};
	const auto validColor = [](const Color& value) {
		return value.r >= 0 && value.r <= 255 && value.g >= 0 &&
		       value.g <= 255 && value.b >= 0 && value.b <= 255 &&
		       value.a >= 0 && value.a <= 255;
	};

	if (style.grid.columns <= 0 || style.grid.rows <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue, base + ".grid");
	}
	if (!fitsGrid(style.grid, style.portrait)) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  base + ".portrait");
	}
	if (!fitsGrid(style.grid, style.text)) {
		throw ConfigError(ConfigErrorKind::InvalidValue, base + ".text");
	}
	if (style.portraitInset < 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  base + ".portrait_inset");
	}
	if (style.portraitScale <= 0.0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  base + ".portrait_scale");
	}
	if (style.textInset < 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  base + ".text_inset");
	}
	if (style.textSize <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  base + ".text_size");
	}
	if (style.lineGap < 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  base + ".line_gap");
	}
	if (!validColor(style.textColor)) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  base + ".text_color");
	}
	return style;
}

std::vector<GridCell> NpcCatalog::readCells(const tomlcfg::TomlDocument& doc,
					    const std::string& path) const {
	std::vector<GridCell> result;
	const std::size_t count = doc.arraySize(path);
	for (std::size_t i = 0; i < count; ++i) {
		const std::string cellPath = path + "[" + std::to_string(i) + "]";
		const std::array<int, 2> cell = doc.getIntArray<2>(cellPath);
		if (cell[0] < 0 || cell[1] < 0) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  cellPath);
		}
		result.push_back(GridCell{cell[0], cell[1]});
	}
	return result;
}

std::vector<std::vector<GridCell>> NpcCatalog::readFrames(
	const tomlcfg::TomlDocument& doc, const std::string& path) const {
	std::vector<std::vector<GridCell>> result;
	const std::size_t count = doc.arraySize(path);
	for (std::size_t i = 0; i < count; ++i) {
		result.push_back(
			readCells(doc, path + "[" + std::to_string(i) + "]"));
	}
	return result;
}

NpcDef NpcCatalog::readNpc(const tomlcfg::TomlDocument& doc,
			   const std::size_t index) const {
	const std::string base = "npcs[" + std::to_string(index) + "].";
	NpcDef npc;
	npc.id = doc.getString(base + "id");
	npc.room = doc.getString(base + "room");
	if (doc.hasKey(base + "text")) {
		npc.text = doc.getString(base + "text");
	}
	if (npc.hasPopup()) {
		const std::string stylePath = base + "style";
		const std::string styleName = doc.hasKey(stylePath)
						      ? doc.getString(stylePath)
						      : kDefaultStyle;
		const auto style = styles_.find(styleName);
		if (style == styles_.end()) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  stylePath);
		}
		npc.popup = style->second;
	}

	const std::string bodyBase = base + "body.";
	npc.body.sheet = doc.getString(bodyBase + "sheet");
	const std::array<int, 2> bodyCell =
		doc.getIntArray<2>(bodyBase + "cell");
	const std::array<int, 2> bodyAt =
		doc.getIntArray<2>(bodyBase + "at");
	const std::array<int, 2> bodySpan =
		doc.getIntArray<2>(bodyBase + "span");
	npc.body.cellWidth = bodyCell[0];
	npc.body.cellHeight = bodyCell[1];
	npc.body.at = GridCell{bodyAt[0], bodyAt[1]};
	npc.body.span = GridSpan{bodySpan[0], bodySpan[1]};
	if (doc.hasKey(bodyBase + "drawn")) {
		const std::array<int, 2> drawn =
			doc.getIntArray<2>(bodyBase + "drawn");
		npc.body.drawn = PixelSize{drawn[0], drawn[1]};
	}
	const std::string framesPath = bodyBase + "frames";
	if (doc.hasKey(framesPath)) {
		npc.body.frames = readFrames(doc, framesPath);
		npc.body.frameTicks = doc.getInt(bodyBase + "frame_ticks");
	}
	if (doc.hasKey(bodyBase + "cells")) {
		npc.body.cells = readCells(doc, bodyBase + "cells");
	} else if (!npc.body.frames.empty()) {
		npc.body.cells = npc.body.frames[0];
	} else {
		throw ConfigError(ConfigErrorKind::MissingStructure,
				  bodyBase + "cells");
	}

	const std::string portraitBase = base + "portrait.";
	if (npc.hasPopup()) {
		npc.portrait.sheet = doc.getString(portraitBase + "sheet");
		const std::array<int, 2> portraitCell =
			doc.getIntArray<2>(portraitBase + "cell");
		const std::array<int, 4> region =
			doc.getIntArray<4>(portraitBase + "region");
		npc.portrait.cellWidth = portraitCell[0];
		npc.portrait.cellHeight = portraitCell[1];
		npc.portrait.at = GridCell{region[0], region[1]};
		npc.portrait.span = GridSpan{region[2], region[3]};
	}

	if (npc.id.empty()) {
		throw ConfigError(ConfigErrorKind::InvalidValue, base + "id");
	}
	if (npc.room.empty()) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  base + "room");
	}
	if (npc.body.sheet.empty()) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  bodyBase + "sheet");
	}
	if (npc.body.cellWidth <= 0 || npc.body.cellHeight <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  bodyBase + "cell");
	}
	if (npc.body.at.column < 0 || npc.body.at.row < 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  bodyBase + "at");
	}
	if (npc.body.span.columns <= 0 || npc.body.span.rows <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  bodyBase + "span");
	}
	if (npc.body.cells.size() != static_cast<std::size_t>(
					    npc.body.span.columns *
					    npc.body.span.rows)) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  bodyBase + "cells");
	}
	if ((npc.body.drawn.width > 0) != (npc.body.drawn.height > 0)) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  bodyBase + "drawn");
	}
	if (!npc.body.frames.empty()) {
		const std::size_t cellCount = static_cast<std::size_t>(
			npc.body.span.columns * npc.body.span.rows);
		for (const std::vector<GridCell>& frame : npc.body.frames) {
			if (frame.size() != cellCount) {
				throw ConfigError(ConfigErrorKind::InvalidValue,
						  framesPath);
			}
		}
		if (npc.body.frames.size() < 2) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  framesPath);
		}
		if (npc.body.frameTicks <= 0) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  bodyBase + "frame_ticks");
		}
	}
	if (!npc.hasPopup()) {
		return npc;
	}
	if (npc.portrait.sheet.empty()) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  portraitBase + "sheet");
	}
	if (npc.portrait.cellWidth <= 0 || npc.portrait.cellHeight <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  portraitBase + "cell");
	}
	if (npc.portrait.at.column < 0 || npc.portrait.at.row < 0 ||
	    npc.portrait.span.columns <= 0 || npc.portrait.span.rows <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  portraitBase + "region");
	}
	return npc;
}

std::vector<const NpcDef*> NpcCatalog::forRoom(
	const std::string& room) const {
	std::vector<const NpcDef*> result;
	for (const NpcDef& npc : npcs_) {
		if (npc.room == room) {
			result.push_back(&npc);
		}
	}
	return result;
}

const NpcDef* NpcCatalog::npcAt(const std::string& room,
				const PixelPoint& point,
				const int cellPixelSize,
				const NpcCells& placed) const {
	for (std::size_t i = npcs_.size(); i > 0; --i) {
		const NpcDef& npc = npcs_[i - 1];
		if (npc.room != room) {
			continue;
		}
		const PixelRect box =
			npc.body.box(cellPixelSize, npc.cellIn(placed));
		if (point.x >= box.x && point.x < box.x + box.width &&
		    point.y >= box.y && point.y < box.y + box.height) {
			return &npc;
		}
	}
	return nullptr;
}

}
