#include "world_model/ScenarioLibraryLoader.h"

#include "world_model/ScenarioSerializer.h"
#include "world_model/SerializerIO.h"

namespace worldmodel {

ScenarioLibrary ScenarioLibraryLoader::loadFromDirectory(
	const std::string& path) const {
	ScenarioLibrary library;
	const ScenarioSerializer serializer;
	for (const std::string& file : SerializerIO::tomlFilesIn(path)) {
		library.add(serializer.loadFromFile(file));
	}
	return library;
}

}
