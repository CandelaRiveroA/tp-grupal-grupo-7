#include "world/CharacterTomlReader.h"

#include "world/ConfigError.h"

namespace world {

namespace {

using Kind = ConfigErrorKind;

constexpr std::array<const char*, 4> kDirectionKeys = {
	"front", "back", "left", "right"};

// Claves del TOML (datos, no lógica; sección 1 del contrato).
constexpr const char* kFrames = "frames";
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
	const std::size_t layers = doc_.arraySize(posePath);
	if (layers == 0) {
		throw ConfigError(Kind::InvalidValue, posePath);
	}
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
	action.frames = readFrames(join(actionPath, kFrames));
	action.loops = doc_.getBool(join(actionPath, kLoops));
	const std::string framePath = join(actionPath, kFrame);
	if (doc_.hasKey(framePath)) {
		action.frameTicks = doc_.getInt(framePath);
		requirePositive(action.frameTicks, framePath);
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
	return action;
}

std::map<ActionKind, ActionData> CharacterTomlReader::readActions(
	const std::string& dirBase, int defaultFrameTicks) const {
	std::map<ActionKind, ActionData> actions;
	for (const auto& [name, kind] : actionNames_.entries()) {
		const std::string actionPath = join(dirBase, name);
		if (doc_.hasKey(actionPath)) {
			actions.emplace(
				kind, readAction(actionPath, defaultFrameTicks));
		}
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
	return hold;
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
