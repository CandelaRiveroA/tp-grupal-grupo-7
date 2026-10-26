#include "world/SceneTomlReader.h"

#include <array>
#include <string>
#include <utility>
#include <vector>

#include "world/ConfigError.h"
#include "world/SceneKeys.h"

namespace world {

SceneTomlReader::SceneTomlReader(const tomlcfg::TomlDocument& doc)
	: doc_(doc) {}

SceneDescription SceneTomlReader::readRoom(
	const std::string& roomName) const {
	SceneDescription scene;
	scene.floorColor = {0, 0, 0, 255};
	scene.grid = {0, 0};

	readTileSets(scene);

	const std::string roomBase = join(kKeyRooms, roomName);
	if (!doc_.hasKey(roomBase)) {
		throw ConfigError(ConfigErrorKind::MissingStructure, roomBase);
	}

	const std::array<int, 4> floor =
		doc_.getIntArray<4>(join(roomBase, kKeyFloorColor));
	scene.floorColor = {floor[0], floor[1], floor[2], floor[3]};

	const std::array<int, 2> grid =
		doc_.getIntArray<2>(join(roomBase, kKeyGrid));
	scene.grid = {grid[0], grid[1]};

	const int defaultScale = doc_.getInt(join(roomBase, kKeyScale));
	if (defaultScale <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  join(roomBase, kKeyScale));
	}

	readElements(roomBase, defaultScale, scene);

	const TileSetDef& referenceTileset =
		scene.tilesets.at(scene.elements.front().tileset);
	scene.cellPixelSize = referenceTileset.cellWidth * defaultScale;

	return scene;
}

void SceneTomlReader::readTileSets(SceneDescription& scene) const {
	const std::vector<std::string> names =
		doc_.subtableKeys(kKeyTilesets);
	for (const std::string& name : names) {
		const std::string base = join(kKeyTilesets, name);
		const std::array<int, 2> cell =
			doc_.getIntArray<2>(join(base, kKeyCell));
		TileSetDef def;
		def.image = doc_.getString(join(base, kKeyImage));
		def.cellWidth = cell[0];
		def.cellHeight = cell[1];
		scene.tilesets.emplace(name, def);
	}
	if (scene.tilesets.empty()) {
		throw ConfigError(ConfigErrorKind::MissingStructure,
				  kKeyTilesets);
	}
}

void SceneTomlReader::readElements(const std::string& roomBase,
				   int defaultScale,
				   SceneDescription& scene) const {
	const std::string elementsPath = join(roomBase, kKeyElements);
	const std::vector<tomlcfg::TomlDocument> elements =
		doc_.getTableArray(elementsPath);
	if (elements.empty()) {
		throw ConfigError(ConfigErrorKind::MissingStructure,
				  elementsPath);
	}

	int elementIndex = 0;
	for (const tomlcfg::TomlDocument& element : elements) {
		const std::string context =
			elementsPath + "[" + std::to_string(elementIndex) +
			"]";

		ElementPlacement placement;
		placement.tileset = element.getString(kKeyTileset);
		if (scene.tilesets.find(placement.tileset) ==
		    scene.tilesets.end()) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  join(context, kKeyTileset));
		}

		const std::array<int, 4> region =
			element.getIntArray<4>(kKeyRegion);
		placement.region.origin = {region[0], region[1]};
		placement.region.size = {region[2], region[3]};

		const std::array<int, 2> at =
			element.getIntArray<2>(kKeyAt);
		placement.at = {at[0], at[1]};

		const std::array<int, 2> span =
			element.getIntArray<2>(kKeySpan);
		placement.span = {span[0], span[1]};

		placement.draw = readDrawSpec(element.getSubtable(kKeyDraw),
					      defaultScale, context);

		scene.elements.push_back(std::move(placement));
		++elementIndex;
	}
}

DrawSpec SceneTomlReader::readDrawSpec(const tomlcfg::TomlDocument& draw,
				       int defaultScale,
				       const std::string& context) const {
	DrawSpec spec;
	spec.mode = parseDrawMode(draw.getString(kKeyMode),
				  join(context, kKeyDraw));
	const int base = draw.hasKey(kKeyScale) ? draw.getInt(kKeyScale)
						: defaultScale;
	spec.scaleX =
		draw.hasKey(kKeyScaleX) ? draw.getInt(kKeyScaleX) : base;
	spec.scaleY =
		draw.hasKey(kKeyScaleY) ? draw.getInt(kKeyScaleY) : base;
	return spec;
}

DrawMode SceneTomlReader::parseDrawMode(
	const std::string& mode, const std::string& context) const {
	if (mode == kDrawModeSingle) {
		return DrawMode::Single;
	}
	if (mode == kDrawModeTile) {
		return DrawMode::Tile;
	}
	if (mode == kDrawModeHorizontalStrip) {
		return DrawMode::HorizontalStrip;
	}
	throw ConfigError(ConfigErrorKind::InvalidValue,
			  join(context, kKeyMode));
}

std::string SceneTomlReader::join(const std::string& base,
				  const std::string& key) const {
	return base + "." + key;
}

}
