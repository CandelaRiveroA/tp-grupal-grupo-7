#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <unordered_set>

#include "toml/TomlDocument.h"
#include "world_model/FieldReader.h"
#include "world_model/PrefabCatalog.h"

namespace worldmodel {

class PrefabCatalogReader {
public:
	explicit PrefabCatalogReader(const tomlcfg::TomlDocument& doc);

	PrefabCatalog read() const;

private:
	std::string readNonEmptyString(const std::string& path) const;
	PrefabDef readPrefab(
		std::size_t index,
		const std::map<std::string, TilesetDef>& tilesets,
		std::unordered_set<std::string>& seenIds) const;

	const tomlcfg::TomlDocument& doc_;
	FieldReader fields_;
};

}
