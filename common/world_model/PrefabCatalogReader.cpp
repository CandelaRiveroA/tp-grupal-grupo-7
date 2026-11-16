#include "world_model/PrefabCatalogReader.h"

#include <array>
#include <optional>

#include "world_model/FormatError.h"

namespace worldmodel {

PrefabCatalogReader::PrefabCatalogReader(const tomlcfg::TomlDocument& doc)
	: doc_(doc), fields_(doc) {}

PrefabCatalog PrefabCatalogReader::read() const {
	fields_.readFormatVersion();

	PrefabCatalog catalog;
	catalog.setTilesets(fields_.readTilesets("tilesets"));
	if (!doc_.hasKey("prefabs")) {
		return catalog;
	}

	const std::size_t count = doc_.arraySize("prefabs");
	catalog.reserve(count);
	std::unordered_set<std::string> seenIds;
	for (std::size_t i = 0; i < count; ++i) {
		catalog.add(readPrefab(i, catalog.tilesets(), seenIds));
	}
	return catalog;
}

std::string PrefabCatalogReader::readNonEmptyString(
	const std::string& path) const {
	std::string value = doc_.getString(path);
	if (value.empty()) {
		throw FormatError(FormatErrorKind::InvalidValue, path);
	}
	return value;
}

PrefabDef PrefabCatalogReader::readPrefab(
	std::size_t index, const std::map<std::string, TilesetDef>& tilesets,
	std::unordered_set<std::string>& seenIds) const {
	const std::string path = fields_.idx("prefabs", index);
	PrefabDef prefab;

	const std::string idPath = fields_.join(path, "id");
	prefab.id = readNonEmptyString(idPath);
	if (!seenIds.insert(prefab.id).second) {
		throw FormatError(FormatErrorKind::InvalidValue, idPath);
	}

	prefab.name = readNonEmptyString(fields_.join(path, "name"));
	prefab.category = readNonEmptyString(fields_.join(path, "category"));
	prefab.categoryLabel =
		readNonEmptyString(fields_.join(path, "category_label"));

	prefab.tile = fields_.readTileRef(fields_.join(path, "tile"), tilesets);

	const std::string sizePath = fields_.join(path, "size");
	const std::array<int, 2> size = doc_.getIntArray<2>(sizePath);
	if (size[0] <= 0 || size[1] <= 0) {
		throw FormatError(FormatErrorKind::InvalidValue, sizePath);
	}
	prefab.width = size[0];
	prefab.height = size[1];

	const std::string gridPath = fields_.join(path, "collision_grid");
	prefab.collisionGrid = doc_.getBoolArray(gridPath);
	const std::size_t expected =
		static_cast<std::size_t>(prefab.width) *
		static_cast<std::size_t>(prefab.height);
	if (prefab.collisionGrid.size() != expected) {
		throw FormatError(FormatErrorKind::InvalidValue, gridPath);
	}
	return prefab;
}

}
