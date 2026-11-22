#include "world/CharacterTomlReader.h"

#include <algorithm>

#include "world/ConfigError.h"

namespace world {

namespace {

using Kind = ConfigErrorKind;

constexpr std::array<const char*, 4> kDirectionKeys = {
	"front", "back", "left", "right"};

// Claves del TOML (datos, no lógica; sección 1 del contrato).
constexpr const char* kFrames = "frames";
constexpr const char* kFramesFrom = "frames_from";
constexpr const char* kReverse = "reverse";
constexpr const char* kLoops = "loops";
constexpr const char* kFrame = "frame";
constexpr const char* kSheet = "sheet";
constexpr const char* kGrid = "grid";
constexpr const char* kDrawn = "drawn";
constexpr const char* kFrameCells = "frame_cells";
constexpr const char* kCadence = "cadence";
constexpr const char* kStep = "step";
constexpr const char* kMirrored = "mirrored";
constexpr const char* kHold = "hold";
constexpr const char* kOffsetY = "offset_y";
constexpr const char* kScale = "scale";
constexpr const char* kContentScale = "content_scale";

// Apariencia personalizable.
constexpr const char* kAppearance = "appearance";
constexpr const char* kSkin = "skin";
constexpr const char* kHair = "hair";
constexpr const char* kShirt = "shirt";
constexpr const char* kPants = "pants";
constexpr const char* kAccessory = "accessory";
constexpr const char* kOption = "option";
constexpr const char* kOffset = "offset";
constexpr const char* kCount = "count";
constexpr const char* kFront = "front";
constexpr const char* kSide = "side";
constexpr const char* kBack = "back";
constexpr const char* kAnchor = "anchor";

constexpr std::array<const char*, 3> kAppearanceSideKeys = {
	kFront, kSide, kBack};

// Anclas de dibujo de una capa, en el orden del enum AppearanceAnchor.
constexpr std::array<const char*, 3> kAppearanceAnchorKeys = {
	"body", "head", "feet"};

// Criatura lógica -> tabla del TOML que la describe.
struct CreatureBinding {
	CreatureKind kind;
	const char* tableKey;
};

constexpr std::array<CreatureBinding, 9> kCreatureBindings = {{
	{CreatureKind::Player, "characters.player"},
	{CreatureKind::Chicken, "characters.chicken"},
	{CreatureKind::Cow, "characters.cow"},
	{CreatureKind::Orc, "characters.orc"},
	{CreatureKind::Skeleton, "characters.skeleton"},
	{CreatureKind::Vampire, "characters.vampire"},
	{CreatureKind::Skull, "characters.skull"},
	{CreatureKind::SkullFire, "characters.skull_fire"},
	{CreatureKind::SkullFireGreen, "characters.skull_fire_green"},
}};

}

CharacterTomlReader::CharacterTomlReader(const tomlcfg::TomlDocument& doc)
	: doc_(doc) {}

std::string CharacterTomlReader::join(const std::string& base,
				      const std::string& key) const {
	return base + "." + key;
}

std::string CharacterTomlReader::idx(const std::string& base,
				     std::size_t i) const {
	return base + "[" + std::to_string(i) + "]";
}

LayerFrame CharacterTomlReader::readLayer(const std::string& posePath,
					  std::size_t layerIdx) const {
	const std::string cellPath = idx(posePath, layerIdx);
	const std::size_t n = doc_.arraySize(cellPath);
	if (n == 2) {
		const std::array<int, 2> a = doc_.getIntArray<2>(cellPath);
		requireNonNegative(a[0], cellPath);
		requireNonNegative(a[1], cellPath);
		return LayerFrame{static_cast<Layer>(layerIdx),
				  SheetCell{a[0], a[1]}, 1, 1};
	}
	if (n == 4) {
		const std::array<int, 4> a = doc_.getIntArray<4>(cellPath);
		requireNonNegative(a[0], cellPath);
		requireNonNegative(a[1], cellPath);
		requirePositive(a[2], cellPath);
		requirePositive(a[3], cellPath);
		return LayerFrame{static_cast<Layer>(layerIdx),
				  SheetCell{a[0], a[1]}, a[2], a[3]};
	}
	throw ConfigError(Kind::InvalidValue, cellPath);
}

Pose CharacterTomlReader::readPose(const std::string& posePath) const {
	// Una pose SIN capas es un frame INVISIBLE (la titilación del daño):
	// no dibuja nada, como los frames sin layers de los overlays. Lo que
	// no puede ser vacía es la lista de frames de la acción.
	const std::size_t layers = doc_.arraySize(posePath);
	Pose pose;
	for (std::size_t i = 0; i < layers; ++i) {
		pose.push_back(readLayer(posePath, i));
	}
	return pose;
}

std::vector<Pose> CharacterTomlReader::readFrames(
	const std::string& framesPath) const {
	const std::size_t count = doc_.arraySize(framesPath);
	if (count == 0) {
		throw ConfigError(Kind::InvalidValue, framesPath);
	}
	std::vector<Pose> frames;
	for (std::size_t i = 0; i < count; ++i) {
		frames.push_back(readPose(idx(framesPath, i)));
	}
	return frames;
}

ActionData CharacterTomlReader::readAction(const std::string& actionPath,
					   int defaultFrameTicks) const {
	ActionData action;
	// `frames_from` hereda los frames de otra acción de la MISMA
	// dirección (la resolución es de readActions, en dos pasadas): en
	// ese caso la acción NO declara `frames`.
	const std::string refPath = join(actionPath, kFramesFrom);
	if (!doc_.hasKey(refPath)) {
		action.frames = readFrames(join(actionPath, kFrames));
	}
	action.loops = doc_.getBool(join(actionPath, kLoops));
	const std::string framePath = join(actionPath, kFrame);
	if (doc_.hasKey(framePath)) {
		action.frameTicks = doc_.getInt(framePath);
		requirePositive(action.frameTicks, framePath);
	} else if (doc_.hasKey(refPath)) {
		// Sin `frame` explícito: 0 = "hereda el ritmo del frames_from"
		// para que copie el de la acción de origen (contrato §10: la
		// ventana y la animación no se desfasan).
		action.frameTicks = 0;
	} else {
		action.frameTicks = defaultFrameTicks;
	}
	// Hoja PROPIA de la acción (opcional): si la declara, se dibuja con
	// esta en vez de la de la criatura. Útil para enemigos con una hoja
	// por acción. `grid` y `frame_cells` acompañan a la hoja.
	const std::string sheetPath = join(actionPath, kSheet);
	if (doc_.hasKey(sheetPath)) {
		action.sheet = doc_.getString(sheetPath);
		if (action.sheet.empty()) {
			throw ConfigError(Kind::InvalidValue, sheetPath);
		}
		const std::string gridPath = join(actionPath, kGrid);
		const std::array<int, 2> grid = doc_.getIntArray<2>(gridPath);
		requirePositive(grid[0], gridPath);
		requirePositive(grid[1], gridPath);
		action.cellWidth = grid[0];
		action.cellHeight = grid[1];
		const std::string frameCellsPath = join(actionPath, kFrameCells);
		if (doc_.hasKey(frameCellsPath)) {
			const std::array<int, 2> fc =
				doc_.getIntArray<2>(frameCellsPath);
			requirePositive(fc[0], frameCellsPath);
			requirePositive(fc[1], frameCellsPath);
			action.frameCols = fc[0];
			action.frameRows = fc[1];
		}
	}
	// `reverse = true`: los frames (propios o heredados) se animan al
	// revés. La inversión se aplica sobre los frames ya armados.
	if (readReverse(actionPath)) {
		std::reverse(action.frames.begin(), action.frames.end());
	}
	return action;
}

bool CharacterTomlReader::readReverse(const std::string& actionPath) const {
	const std::string reversePath = join(actionPath, kReverse);
	return doc_.hasKey(reversePath) && doc_.getBool(reversePath);
}

void CharacterTomlReader::applyFramesFrom(
	std::map<ActionKind, ActionData>& actions, const ActionKind kind,
	const std::string& refName, const std::string& actionPath) const {
	const std::string refPath = join(actionPath, kFramesFrom);
	const ActionKind* refKind = actionNames_.find(refName);
	if (refKind == nullptr) {
		throw ConfigError(Kind::InvalidValue, refPath);
	}
	const auto source = actions.find(*refKind);
	if (source == actions.end() || source->second.frames.empty()) {
		throw ConfigError(Kind::InvalidValue, refPath);
	}
	// Hereda los frames y, con ellos, la hoja propia de la acción de
	// origen (sheet/grid/frame_cells) y su ritmo. Un `frame` explícito
	// en la acción que hereda se respeta.
	ActionData& target = actions.at(kind);
	target.frames = source->second.frames;
	if (target.frameTicks == 0) {
		target.frameTicks = source->second.frameTicks;
	}
	if (target.sheet.empty()) {
		target.sheet = source->second.sheet;
		target.cellWidth = source->second.cellWidth;
		target.cellHeight = source->second.cellHeight;
		target.frameCols = source->second.frameCols;
		target.frameRows = source->second.frameRows;
	}
	if (readReverse(actionPath)) {
		std::reverse(target.frames.begin(), target.frames.end());
	}
}

std::map<ActionKind, ActionData> CharacterTomlReader::readActions(
	const std::string& dirBase, int defaultFrameTicks) const {
	std::map<ActionKind, ActionData> actions;
	// Referencias frames_from pendientes (acción -> nombre de origen):
	// se resuelven en una SEGUNDA pasada para no depender del orden en
	// que se lean las acciones de la dirección.
	std::map<ActionKind, std::pair<std::string, std::string>> pendingRefs;
	for (const auto& [name, kind] : actionNames_.entries()) {
		const std::string actionPath = join(dirBase, name);
		if (!doc_.hasKey(actionPath)) {
			continue;
		}
		const std::string refPath = join(actionPath, kFramesFrom);
		if (doc_.hasKey(refPath)) {
			pendingRefs.emplace(
				kind, std::make_pair(doc_.getString(refPath),
						     actionPath));
		}
		actions.emplace(
			kind, readAction(actionPath, defaultFrameTicks));
	}
	for (const auto& [kind, ref] : pendingRefs) {
		applyFramesFrom(actions, kind, ref.first, ref.second);
	}
	return actions;
}

DirectionAppearance CharacterTomlReader::readDirection(
	const std::string& dirBase, int defaultFrameTicks) const {
	DirectionAppearance direction;
	direction.mirrored = doc_.getBool(join(dirBase, kMirrored));
	direction.actions = readActions(dirBase, defaultFrameTicks);
	return direction;
}

CharacterCatalog::CreatureEntry CharacterTomlReader::readCreature(
	const std::string& base) const {
	CharacterCatalog::CreatureEntry entry;

	const std::string sheetPath = join(base, kSheet);
	entry.sheetFile = doc_.getString(sheetPath);
	if (entry.sheetFile.empty()) {
		throw ConfigError(Kind::InvalidValue, sheetPath);
	}

	const std::string gridPath = join(base, kGrid);
	const std::array<int, 2> grid = doc_.getIntArray<2>(gridPath);
	requirePositive(grid[0], gridPath);
	requirePositive(grid[1], gridPath);
	entry.sheetCellWidth = grid[0];
	entry.sheetCellHeight = grid[1];

	const std::string drawnPath = join(base, kDrawn);
	const std::array<int, 2> drawn = doc_.getIntArray<2>(drawnPath);
	requirePositive(drawn[0], drawnPath);
	requirePositive(drawn[1], drawnPath);
	entry.drawnWidth = drawn[0];
	entry.drawnHeight = drawn[1];

	// Opcional: el frame de referencia en celdas (la escala fija de la
	// criatura); sin la clave, cada frame se escala por sus celdas.
	const std::string frameCellsPath = join(base, kFrameCells);
	if (doc_.hasKey(frameCellsPath)) {
		const std::array<int, 2> frameCells =
			doc_.getIntArray<2>(frameCellsPath);
		requirePositive(frameCells[0], frameCellsPath);
		requirePositive(frameCells[1], frameCellsPath);
		entry.frameCols = frameCells[0];
		entry.frameRows = frameCells[1];
	}

	entry.data.cadence = readCadence(base);
	entry.data.hold = readHold(base);
	entry.data.directions = readDirections(base, entry.data.cadence);
	entry.appearance = readAppearance(join(base, kAppearance));
	return entry;
}

HoldPresentation CharacterTomlReader::readHold(const std::string& base) const {
	HoldPresentation hold;
	const std::string holdBase = join(base, kHold);
	if (!doc_.hasKey(holdBase)) {
		return hold;
	}
	const std::string offsetPath = join(holdBase, kOffsetY);
	if (doc_.hasKey(offsetPath)) {
		// Con signo (como los offsets de tools_overlay.toml):
		// positivo separa el ítem sobre la cabeza; negativo lo baja
		// y pisa la cabeza. Es puro ajuste visual.
		hold.offsetY = doc_.getInt(offsetPath);
	}
	const std::string scalePath = join(holdBase, kScale);
	if (doc_.hasKey(scalePath)) {
		const double scale = doc_.getDouble(scalePath);
		if (scale <= 0.0) {
			throw ConfigError(Kind::InvalidValue, scalePath);
		}
		hold.scale = scale;
	}
	const std::string contentBase = join(holdBase, kContentScale);
	if (doc_.hasKey(contentBase)) {
		for (const std::string& id : doc_.subtableKeys(contentBase)) {
			const std::string path = join(contentBase, id);
			const double factor = doc_.getDouble(path);
			if (factor <= 0.0) {
				throw ConfigError(Kind::InvalidValue, path);
			}
			hold.contentScale.emplace(id, factor);
		}
	}
	return hold;
}

std::vector<int> CharacterTomlReader::readInts(
	const std::string& path) const {
	const std::size_t count = doc_.arraySize(path);
	std::vector<int> values;
	values.reserve(count);
	for (std::size_t i = 0; i < count; ++i) {
		values.push_back(doc_.getInt(idx(path, i)));
	}
	return values;
}

std::vector<SheetRegion> CharacterTomlReader::readAppearanceRegions(
	const std::string& path) const {
	const std::vector<int> values = readInts(path);
	if (values.empty() || values.size() % 4 != 0) {
		throw ConfigError(Kind::InvalidValue, path);
	}
	std::vector<SheetRegion> regions;
	for (std::size_t i = 0; i < values.size(); i += 4) {
		const int col = values[i];
		const int row = values[i + 1];
		const int cols = values[i + 2];
		const int rows = values[i + 3];
		requireNonNegative(col, path);
		requireNonNegative(row, path);
		requirePositive(cols, path);
		requirePositive(rows, path);
		regions.push_back(
			SheetRegion(SheetCell(col, row), cols, rows));
	}
	return regions;
}

AppearanceOption CharacterTomlReader::readAppearanceOption(
	const std::string& optionPath) const {
	AppearanceOption option;
	for (std::size_t i = 0; i < kAppearanceSideKeys.size(); ++i) {
		const std::string sidePath =
			join(optionPath, kAppearanceSideKeys[i]);
		if (doc_.hasKey(sidePath)) {
			option.sides[i] = readAppearanceRegions(sidePath);
		}
	}
	return option;
}

AppearanceLayerData CharacterTomlReader::readAppearanceLayer(
	const std::string& base) const {
	AppearanceLayerData layer;

	const std::string sheetPath = join(base, kSheet);
	layer.sheetImage = doc_.getString(sheetPath);
	if (layer.sheetImage.empty()) {
		throw ConfigError(Kind::InvalidValue, sheetPath);
	}

	const std::string gridPath = join(base, kGrid);
	const std::array<int, 2> grid = doc_.getIntArray<2>(gridPath);
	requirePositive(grid[0], gridPath);
	requirePositive(grid[1], gridPath);
	layer.cellWidth = grid[0];
	layer.cellHeight = grid[1];

	const std::string offsetPath = join(base, kOffset);
	if (doc_.hasKey(offsetPath)) {
		// Con signo: las capas ancladas a la coronilla se posicionan
		// sobre ella (offset.y negativo = sobre la cabeza).
		const std::array<int, 2> offset =
			doc_.getIntArray<2>(offsetPath);
		for (std::array<int, 2>& face : layer.offset) {
			face = offset;
		}
	}
	// Override por cara (opcional): `offset_front/side/back` ajustan una
	// dirección concreta sobre el offset base.
	for (std::size_t face = 0; face < kAppearanceSideKeys.size(); ++face) {
		const std::string path = join(
			base, std::string(kOffset) + "_" + kAppearanceSideKeys[face]);
		if (doc_.hasKey(path)) {
			layer.offset[face] = doc_.getIntArray<2>(path);
		}
	}

	// Ancla de dibujo del offset: `body` (por defecto) o `head`.
	const std::string anchorPath = join(base, kAnchor);
	if (doc_.hasKey(anchorPath)) {
		const std::string value = doc_.getString(anchorPath);
		bool found = false;
		for (std::size_t i = 0; i < kAppearanceAnchorKeys.size(); ++i) {
			if (value == kAppearanceAnchorKeys[i]) {
				layer.anchor = static_cast<AppearanceAnchor>(i);
				found = true;
				break;
			}
		}
		if (!found) {
			throw ConfigError(Kind::InvalidValue, anchorPath);
		}
	}

	const std::string optionsPath = join(base, kOption);
	if (!doc_.hasKey(optionsPath)) {
		throw ConfigError(Kind::MissingStructure, optionsPath);
	}
	const std::size_t count = doc_.arraySize(optionsPath);
	if (count == 0) {
		throw ConfigError(Kind::InvalidValue, optionsPath);
	}
	for (std::size_t i = 0; i < count; ++i) {
		layer.options.push_back(
			readAppearanceOption(idx(optionsPath, i)));
	}
	return layer;
}

SkinAppearanceData CharacterTomlReader::readSkinAppearance(
	const std::string& base) const {
	SkinAppearanceData skin;

	const std::string sheetPath = join(base, kSheet);
	skin.paletteSheet = doc_.getString(sheetPath);
	if (skin.paletteSheet.empty()) {
		throw ConfigError(Kind::InvalidValue, sheetPath);
	}

	const std::string gridPath = join(base, kGrid);
	const std::array<int, 2> grid = doc_.getIntArray<2>(gridPath);
	requirePositive(grid[0], gridPath);
	requirePositive(grid[1], gridPath);
	skin.cellWidth = grid[0];
	skin.cellHeight = grid[1];

	const std::string countPath = join(base, kCount);
	skin.count = doc_.getInt(countPath);
	requirePositive(skin.count, countPath);
	return skin;
}

AppearanceData CharacterTomlReader::readAppearance(
	const std::string& base) const {
	AppearanceData appearance;
	if (!doc_.hasKey(base)) {
		return appearance;
	}
	appearance.present = true;
	appearance.skin = readSkinAppearance(join(base, kSkin));
	appearance.hair = readAppearanceLayer(join(base, kHair));
	appearance.shirt = readAppearanceLayer(join(base, kShirt));
	appearance.pants = readAppearanceLayer(join(base, kPants));
	appearance.accessory =
		readAppearanceLayer(join(base, kAccessory));
	return appearance;
}

MoveCadence CharacterTomlReader::readCadence(const std::string& base) const {
	const std::string cadenceBase = join(base, kCadence);
	const std::string stepPath = join(cadenceBase, kStep);
	const std::string framePath = join(cadenceBase, kFrame);
	const int step = doc_.getInt(stepPath);
	const int frame = doc_.getInt(framePath);
	requirePositive(frame, framePath);
	requirePositive(step, stepPath);
	if (step % frame != 0) {
		throw ConfigError(Kind::InvalidValue, cadenceBase);
	}
	return MoveCadence{step, frame};
}

std::array<DirectionAppearance, 4>
CharacterTomlReader::readDirections(const std::string& base,
				    const MoveCadence& cadence) const {
	std::array<DirectionAppearance, 4> directions;
	for (std::size_t i = 0; i < kDirectionKeys.size(); ++i) {
		const std::string dirBase = join(base, kDirectionKeys[i]);
		directions[i] = readDirection(dirBase, cadence.frameTicks);
		if (directions[i].actions.find(ActionKind::Idle) ==
		    directions[i].actions.end()) {
			throw ConfigError(Kind::MissingStructure,
					  join(dirBase, *actionNames_.nameFor(
								ActionKind::Idle)));
		}
		const auto walk = directions[i].actions.find(ActionKind::Walk);
		if (walk == directions[i].actions.end()) {
			throw ConfigError(Kind::MissingStructure,
					  join(dirBase, *actionNames_.nameFor(
								ActionKind::Walk)));
		}
		const int frames =
			static_cast<int>(walk->second.frames.size());
		if (cadence.stepTicks != walk->second.frameTicks * frames) {
			throw ConfigError(Kind::InvalidValue,
					  join(dirBase, *actionNames_.nameFor(
								ActionKind::Walk)));
		}
	}
	return directions;
}

void CharacterTomlReader::requirePositive(int value,
					  const std::string& path) const {
	if (value <= 0) {
		throw ConfigError(Kind::InvalidValue, path);
	}
}

void CharacterTomlReader::requireNonNegative(int value,
					     const std::string& path) const {
	if (value < 0) {
		throw ConfigError(Kind::InvalidValue, path);
	}
}

CharacterCatalog CharacterTomlReader::readCatalog() const {
	CharacterCatalog catalog;
	mergeInto(catalog);
	return catalog;
}

void CharacterTomlReader::mergeInto(CharacterCatalog& catalog) const {
	// Lee SOLO las tablas presentes en este documento: el mismo reader
	// sirve para characters.toml (jugador y animales) y enemies.toml
	// (enemigos), volcando sobre el MISMO catálogo. El binding
	// nombre->kind es el vocabulario; una tabla ausente no se carga
	// (otro archivo la trae).
	for (const CreatureBinding& binding : kCreatureBindings) {
		if (doc_.hasKey(binding.tableKey)) {
			catalog.setEntry(binding.kind,
					 readCreature(binding.tableKey));
		}
	}
}

}
