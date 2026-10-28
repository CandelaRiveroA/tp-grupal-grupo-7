#include "world/CharacterCatalogLoader.h"

#include <array>
#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "toml/TomlDocument.h"
#include "world/CharacterAppearance.h"
#include "world/ConfigError.h"
#include "world/CreatureCatalog.h"

namespace world {

namespace {

using tomlcfg::TomlDocument;
using tomlcfg::TomlError;
using Kind = ConfigErrorKind;

constexpr std::array<const char*, 4> kDirectionKeys = {
	"front", "back", "left", "right"};

struct CreatureBinding {
	CreatureKind kind;
	const char* tableKey;
};

constexpr std::array<CreatureBinding, 3> kCreatureBindings = {{
	{CreatureKind::Player, "characters.player"},
	{CreatureKind::Chicken, "characters.chicken"},
	{CreatureKind::Cow, "characters.cow"},
}};

class CharacterTomlReader {
public:
	explicit CharacterTomlReader(const TomlDocument& doc) : doc_(doc) {}

	CharacterCatalog readCatalog() const {
		CharacterCatalog catalog;
		for (const CreatureBinding& binding : kCreatureBindings) {
			catalog.setEntry(binding.kind,
					 readCreature(binding.tableKey));
		}
		return catalog;
	}

private:
	static std::string join(const std::string& base,
				const std::string& key) {
		return base + "." + key;
	}

	static std::string idx(const std::string& base, std::size_t i) {
		return base + "[" + std::to_string(i) + "]";
	}

	LayerFrame readLayer(const std::string& posePath,
			     std::size_t layerIdx) const {
		const std::string cellPath = idx(posePath, layerIdx);
		const std::size_t n = doc_.arraySize(cellPath);
		if (n == 2) {
			const std::array<int, 2> a =
				doc_.getIntArray<2>(cellPath);
			requireNonNegative(a[0], cellPath);
			requireNonNegative(a[1], cellPath);
			return LayerFrame{static_cast<Layer>(layerIdx),
					  FrameIndex{a[0], a[1]}, 1, 1};
		}
		if (n == 4) {
			const std::array<int, 4> a =
				doc_.getIntArray<4>(cellPath);
			requireNonNegative(a[0], cellPath);
			requireNonNegative(a[1], cellPath);
			requirePositive(a[2], cellPath);
			requirePositive(a[3], cellPath);
			return LayerFrame{static_cast<Layer>(layerIdx),
					  FrameIndex{a[0], a[1]}, a[2], a[3]};
		}
		throw ConfigError(Kind::InvalidValue, cellPath);
	}

	Pose readPose(const std::string& posePath) const {
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

	std::vector<Pose> readFrames(const std::string& framesPath) const {
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

	ActionData readAction(const std::string& actionPath,
			      int defaultFrameTicks) const {
		ActionData action;
		action.frames = readFrames(join(actionPath, "frames"));
		action.loops = doc_.getBool(join(actionPath, "loops"));
		const std::string framePath = join(actionPath, "frame");
		if (doc_.hasKey(framePath)) {
			action.frameTicks = doc_.getInt(framePath);
			requirePositive(action.frameTicks, framePath);
		} else {
			action.frameTicks = defaultFrameTicks;
		}
		return action;
	}

	std::map<std::string, ActionKind> actionNames() const {
		return {
			{"idle", ActionKind::Idle},
			{"walk", ActionKind::Walk},
			{"use_tool", ActionKind::UseTool},
		};
	}

	std::map<ActionKind, ActionData> readActions(
		const std::string& dirBase, int defaultFrameTicks) const {
		std::map<ActionKind, ActionData> actions;
		for (const auto& [name, kind] : actionNames()) {
			const std::string actionPath = join(dirBase, name);
			if (doc_.hasKey(actionPath)) {
				actions.emplace(
					kind,
					readAction(actionPath, defaultFrameTicks));
			}
		}
		return actions;
	}

	CharacterCatalog::DirectionData readDirection(
		const std::string& dirBase, int defaultFrameTicks) const {
		CharacterCatalog::DirectionData direction;
		direction.mirrored = doc_.getBool(join(dirBase, "mirrored"));
		direction.actions = readActions(dirBase, defaultFrameTicks);
		return direction;
	}

	CharacterCatalog::CreatureEntry readCreature(
		const std::string& base) const {
		CharacterCatalog::CreatureEntry entry;

		const std::string sheetPath = join(base, "sheet");
		entry.sheetFile = doc_.getString(sheetPath);
		if (entry.sheetFile.empty()) {
			throw ConfigError(Kind::InvalidValue, sheetPath);
		}

		const std::string gridPath = join(base, "grid");
		const std::array<int, 2> grid = doc_.getIntArray<2>(gridPath);
		requirePositive(grid[0], gridPath);
		requirePositive(grid[1], gridPath);
		entry.sheetCellWidth = grid[0];
		entry.sheetCellHeight = grid[1];

		const std::string drawnPath = join(base, "drawn");
		const std::array<int, 2> drawn =
			doc_.getIntArray<2>(drawnPath);
		requirePositive(drawn[0], drawnPath);
		requirePositive(drawn[1], drawnPath);
		entry.drawnWidth = drawn[0];
		entry.drawnHeight = drawn[1];

		entry.cadence = readCadence(base);
		entry.directionData = readDirections(base, entry.cadence);
		return entry;
	}

	MoveCadence readCadence(const std::string& base) const {
		const std::string cadenceBase = join(base, "cadence");
		const std::string stepPath = join(cadenceBase, "step");
		const std::string framePath = join(cadenceBase, "frame");
		const int step = doc_.getInt(stepPath);
		const int frame = doc_.getInt(framePath);
		requirePositive(frame, framePath);
		requirePositive(step, stepPath);
		if (step % frame != 0) {
			throw ConfigError(Kind::InvalidValue, cadenceBase);
		}
		return MoveCadence{step, frame};
	}

	std::array<CharacterCatalog::DirectionData, 4> readDirections(
		const std::string& base, const MoveCadence& cadence) const {
		std::array<CharacterCatalog::DirectionData, 4> directions;
		for (std::size_t i = 0; i < kDirectionKeys.size(); ++i) {
			const std::string dirBase =
				join(base, kDirectionKeys[i]);
			directions[i] =
				readDirection(dirBase, cadence.frameTicks);
			if (directions[i].actions.find(ActionKind::Idle) ==
			    directions[i].actions.end()) {
				throw ConfigError(Kind::MissingStructure,
						  join(dirBase, "idle"));
			}
			const auto walk =
				directions[i].actions.find(ActionKind::Walk);
			if (walk == directions[i].actions.end()) {
				throw ConfigError(Kind::MissingStructure,
						  join(dirBase, "walk"));
			}
			const int frames =
				static_cast<int>(walk->second.frames.size());
			if (cadence.stepTicks !=
			    walk->second.frameTicks * frames) {
				throw ConfigError(Kind::InvalidValue,
						  join(dirBase, "walk"));
			}
		}
		return directions;
	}

	void requirePositive(int value, const std::string& path) const {
		if (value <= 0) {
			throw ConfigError(Kind::InvalidValue, path);
		}
	}

	void requireNonNegative(int value, const std::string& path) const {
		if (value < 0) {
			throw ConfigError(Kind::InvalidValue, path);
		}
	}

	const TomlDocument& doc_;
};

}

CharacterCatalog CharacterCatalogLoader::loadFromString(
	const std::string& toml) const {
	try {
		const TomlDocument doc(toml, TomlDocument::Source::Text);
		return CharacterTomlReader(doc).readCatalog();
	} catch (const TomlError& error) {
		throw ConfigError(error);
	}
}

CharacterCatalog CharacterCatalogLoader::loadFromFile(
	const std::string& path) const {
	try {
		const TomlDocument doc(path, TomlDocument::Source::File);
		return CharacterTomlReader(doc).readCatalog();
	} catch (const TomlError& error) {
		throw ConfigError(error);
	}
}

}
