#include "world_model/PrefabCatalogLoader.h"

#include "toml/TomlDocument.h"
#include "world_model/PrefabCatalogReader.h"
#include "world_model/SerializerIO.h"

namespace worldmodel {

PrefabCatalog PrefabCatalogLoader::loadFromFile(
	const std::string& path) const {
	return SerializerIO::load<PrefabCatalog, PrefabCatalogReader>(
		path, tomlcfg::TomlDocument::Source::File);
}

}
