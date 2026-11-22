#include "world/SceneTomlReader.h"

#include <array>
#include <string>
#include <utility>
#include <vector>

#include "world/ConfigError.h"
#include "world/SceneKeys.h"
#include "world/SpriteVisual.h"

namespace world {

SceneTomlReader::SceneTomlReader(const tomlcfg::TomlDocument& doc)
	: directionsByName_{
		  {"front", Direction::Front},
		  {"back", Direction::Back},
		  {"left", Direction::Left},
		  {"right", Direction::Right},
	  }
	, timeOfDayByName_{
		  {kTimeOfDayDay, TimeOfDay::Day},
		  {kTimeOfDayNight, TimeOfDay::Night},
	  }
	, doc_(doc) {}

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

	// Cues de sonido de la sala (OPCIONAL, sin la key la sala es
	// muda)
	if (doc_.hasKey(join(roomBase, kKeySounds))) {
		scene.soundCues =
			doc_.getStringArray(join(roomBase, kKeySounds));
	}

	readSpawns(roomBase, scene);

	readElements(roomBase, defaultScale, scene);

	const TileSetDef& referenceTileset =
		scene.tilesets.at(scene.elements.front().tileset);
	scene.cellPixelSize = referenceTileset.cellWidth * defaultScale;

	return scene;
}

void SceneTomlReader::readTileSetEntries(SceneDescription& scene) const {
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
}

void SceneTomlReader::readExtraTileSets(SceneDescription& scene) const {
	if (!doc_.hasKey(kKeyTilesets)) {
		return;
	}
	readTileSetEntries(scene);
}

void SceneTomlReader::readExtraElements(const std::string& base,
					int defaultScale,
					SceneDescription& scene) const {
	if (!doc_.hasKey(join(base, kKeyElements))) {
		return;
	}
	readElements(base, defaultScale, scene);
}

void SceneTomlReader::readTileSets(SceneDescription& scene) const {
	readTileSetEntries(scene);
	if (scene.tilesets.empty()) {
		throw ConfigError(ConfigErrorKind::MissingStructure,
				  kKeyTilesets);
	}
}

void SceneTomlReader::readSpawns(const std::string& roomBase,
				 SceneDescription& scene) const {
	// Poblaciones de la sala (OPCIONAL, cada bandera OPCIONAL): sin la
	// subtabla la sala no puebla nada (el caso del dormitorio). El dato
	// vive donde el servidor lo mandará mañana; el dibujo no lo mira.
	const std::string base = join(roomBase, kKeySpawns);
	if (!doc_.hasKey(base)) {
		return;
	}
	const std::string animals = join(base, kKeySpawnAnimals);
	if (doc_.hasKey(animals)) {
		scene.spawns.animals = doc_.getBool(animals);
	}
	const std::string minerals = join(base, kKeySpawnMinerals);
	if (doc_.hasKey(minerals)) {
		scene.spawns.minerals = doc_.getBool(minerals);
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

		readPiece(element, context, placement);

		const std::array<int, 2> at =
			element.getIntArray<2>(kKeyAt);
		placement.at = {at[0], at[1]};

		const std::array<int, 2> span =
			element.getIntArray<2>(kKeySpan);
		placement.span = {span[0], span[1]};

		placement.draw = readDrawSpec(element.getSubtable(kKeyDraw),
					      defaultScale, context);

		readEnter(element, context, placement);

		readWhen(element, context, placement);

		scene.elements.push_back(std::move(placement));
		++elementIndex;
	}
}

void SceneTomlReader::readPiece(const tomlcfg::TomlDocument& element,
				const std::string& context,
				ElementPlacement& placement) const {
	// Dos formas de describir la pieza, exactamente una:
	//   region = [col, row, cols, rows]  -> bloque contiguo de la hoja
	//   piece  = [cols, rows] + cells = [[col, row], ...] -> composición
	//        explícita, celda por celda (fila por fila): cualquier
	//        pieza de cualquier hoja, de cualquier tamaño.
	// Ambas se normalizan a tamaño + una celda de hoja por posición:
	// el dibujo tiene un solo camino y el formato escala solo.
	const bool hasRegion = element.hasKey(kKeyRegion);
	const bool hasCells = element.hasKey(kKeyCells);
	if (hasRegion == hasCells) {
		// ni una forma ni la otra (o las dos a la vez): error
		throw ConfigError(ConfigErrorKind::MissingStructure,
				  join(context, kKeyRegion));
	}

	if (hasCells) {
		const std::array<int, 2> piece =
			element.getIntArray<2>(kKeyPiece);
		if (piece[0] <= 0 || piece[1] <= 0) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  join(context, kKeyPiece));
		}
		placement.pieceSize = {piece[0], piece[1]};
		const std::size_t count = element.arraySize(kKeyCells);
		if (count !=
		    static_cast<std::size_t>(piece[0] * piece[1])) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  join(context, kKeyCells));
		}
		for (std::size_t i = 0; i < count; ++i) {
			const SheetCell cell(element, idx(kKeyCells, i));
			placement.pieceCells.push_back(
				{cell.col, cell.row});
		}
		return;
	}

	const std::array<int, 4> region =
		element.getIntArray<4>(kKeyRegion);
	if (region[0] < 0 || region[1] < 0 || region[2] <= 0 ||
	    region[3] <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  join(context, kKeyRegion));
	}
	placement.pieceSize = {region[2], region[3]};
	for (int row = 0; row < region[3]; ++row) {
		for (int column = 0; column < region[2]; ++column) {
			placement.pieceCells.push_back(
				{region[0] + column, region[1] + row});
		}
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

void SceneTomlReader::readEnter(const tomlcfg::TomlDocument& element,
				const std::string& context,
				ElementPlacement& placement) const {
	// "Entrar" al elemento (OPCIONAL): sin la subtabla el elemento es
	// decoración pura. Con ella, el jugador puede meterse: las filas
	// `rows` de la pieza se dibujan encima de las entidades y él queda
	// en `lie`, mirando `facing` (presentación local, ver
	// ElementEnter).
	if (!element.hasKey(kKeyEnter)) {
		return;
	}
	const std::string enterPath = join(context, kKeyEnter);
	const tomlcfg::TomlDocument enter = element.getSubtable(kKeyEnter);

	const std::array<int, 2> rows =
		enter.getIntArray<2>(kKeyEnterRows);
	if (rows[0] < 0 || rows[1] < rows[0] ||
	    rows[1] >= placement.pieceSize.rows) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  join(enterPath, kKeyEnterRows));
	}

	// La celda donde se mete el jugador está dentro del área que
	// ocupa el elemento en la grilla lógica.
	const std::array<int, 2> lie = enter.getIntArray<2>(kKeyEnterLie);
	if (lie[0] < placement.at.column ||
	    lie[0] >= placement.at.column + placement.span.columns ||
	    lie[1] < placement.at.row ||
	    lie[1] >= placement.at.row + placement.span.rows) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  join(enterPath, kKeyEnterLie));
	}

	placement.enter =
		ElementEnter{rows[0], rows[1], {lie[0], lie[1]},
			     parseFacing(enter.getString(kKeyEnterFacing),
					 enterPath)};
}

Direction SceneTomlReader::parseFacing(const std::string& name,
				       const std::string& context) const {
	const std::map<std::string, Direction>::const_iterator found =
		directionsByName_.find(name);
	if (found == directionsByName_.end()) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  join(context, kKeyEnterFacing));
	}
	return found->second;
}

void SceneTomlReader::readWhen(const tomlcfg::TomlDocument& element,
			       const std::string& context,
			       ElementPlacement& placement) const {
	// Momento del día (OPCIONAL): sin la key el elemento se dibuja
	// en cualquier momento (el caso de toda sala hasta ahora). Con
	// ella, el render lo filtra con visibleAt.
	if (!element.hasKey(kKeyWhen)) {
		return;
	}
	placement.when =
		parseTimeOfDay(element.getString(kKeyWhen),
			       join(context, kKeyWhen));
}

TimeOfDay SceneTomlReader::parseTimeOfDay(const std::string& name,
					  const std::string& context) const {
	const std::map<std::string, TimeOfDay>::const_iterator found =
		timeOfDayByName_.find(name);
	if (found == timeOfDayByName_.end()) {
		throw ConfigError(ConfigErrorKind::InvalidValue, context);
	}
	return found->second;
}

std::string SceneTomlReader::join(const std::string& base,
				  const std::string& key) const {
	return base + "." + key;
}

std::string SceneTomlReader::idx(const std::string& base,
				 const std::size_t i) const {
	return base + "[" + std::to_string(i) + "]";
}

}
