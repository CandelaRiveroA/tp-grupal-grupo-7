#include "world/ResourceTomlReader.h"

#include <array>

#include "world/ConfigError.h"

namespace world {

namespace {

using tomlcfg::TomlDocument;
using Kind = ConfigErrorKind;

// Claves del TOML.
constexpr const char* kOffsetY = "offset_y";
constexpr const char* kScale = "scale";
constexpr const char* kSheet = "sheet";
constexpr const char* kCell = "cell";
constexpr const char* kIntact = "intact";
constexpr const char* kCracked = "cracked";
constexpr const char* kBroken = "broken";

// Recurso lógico -> tabla del TOML que lo describe.
struct ResourceBinding {
	ResourceKind kind;
	const char* tableKey;
};

constexpr std::array<ResourceBinding, 4> kResourceBindings = {{
	{ResourceKind::Silver, "silver"},
	{ResourceKind::Coal, "coal"},
	{ResourceKind::Lapis, "lapis"},
	{ResourceKind::Gold, "gold"},
}};

}

ResourceTomlReader::ResourceTomlReader(const TomlDocument& doc) : doc_(doc) {}

std::string ResourceTomlReader::join(const std::string& base,
				     const std::string& key) const {
	return base + "." + key;
}

std::map<std::string, ResourceState> ResourceTomlReader::stateNames() const {
	return {
		{kIntact, ResourceState::Intact},
		{kCracked, ResourceState::Cracked},
		{kBroken, ResourceState::Broken},
	};
}

ResourceVisual ResourceTomlReader::readVisual(const std::string& base,
					      const std::string& sheet,
					      int cellWidth, int cellHeight,
					      const std::string& statePath) const {
	const std::string path = join(base, statePath);
	const std::array<int, 4> block = doc_.getIntArray<4>(path);
	if (block[0] < 0 || block[1] < 0 || block[2] <= 0 || block[3] <= 0) {
		throw ConfigError(Kind::InvalidValue, path);
	}
	ResourceVisual visual;
	visual.sprite.sheetImage = sheet;
	visual.sprite.cellWidth = cellWidth;
	visual.sprite.cellHeight = cellHeight;
	visual.sprite.col = block[0];
	visual.sprite.row = block[1];
	visual.cols = block[2];
	visual.rows = block[3];
	return visual;
}

void ResourceTomlReader::readResource(ResourceKind kind,
				      const std::string& base,
				      ResourceCatalog& catalog) const {
	const std::string sheetPath = join(base, kSheet);
	const std::string sheet = doc_.getString(sheetPath);
	if (sheet.empty()) {
		throw ConfigError(Kind::InvalidValue, sheetPath);
	}

	const std::string cellPath = join(base, kCell);
	const std::array<int, 2> cell = doc_.getIntArray<2>(cellPath);
	if (cell[0] <= 0 || cell[1] <= 0) {
		throw ConfigError(Kind::InvalidValue, cellPath);
	}

	std::map<ResourceState, ResourceVisual> byState;
	for (const auto& [name, state] : stateNames()) {
		byState.emplace(state, readVisual(base, sheet, cell[0],
						  cell[1], name));
	}
	catalog.visuals.emplace(kind, std::move(byState));
}

ResourceCatalog ResourceTomlReader::read() const {
	ResourceCatalog catalog;
	if (doc_.hasKey(kOffsetY)) {
		catalog.offsetY = doc_.getInt(kOffsetY);
	}
	if (doc_.hasKey(kScale)) {
		catalog.scale = doc_.getDouble(kScale);
	}
	for (const ResourceBinding& binding : kResourceBindings) {
		if (!doc_.hasKey(binding.tableKey)) {
			continue;
		}
		readResource(binding.kind, binding.tableKey, catalog);
	}
	return catalog;
}

}
