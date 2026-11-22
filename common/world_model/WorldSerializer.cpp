#include "world_model/WorldSerializer.h"

#include "toml/TomlDocument.h"
#include "world_model/SerializerIO.h"
#include "world_model/WorldTomlReader.h"
#include "world_model/WorldTomlWriter.h"

namespace worldmodel {

World WorldSerializer::loadFromFile(const std::string& path) const {
	return SerializerIO::load<World, WorldTomlReader>(
		path, tomlcfg::TomlDocument::Source::File);
}

World WorldSerializer::loadFromString(const std::string& toml) const {
	return SerializerIO::load<World, WorldTomlReader>(
		toml, tomlcfg::TomlDocument::Source::Text);
}

void WorldSerializer::saveToFile(const World& world,
				 const std::string& path) const {
	SerializerIO::save(saveToString(world), path);
}

std::string WorldSerializer::saveToString(const World& world) const {
	return WorldTomlWriter().write(world);
}

}
