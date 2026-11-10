#include "world_model/ScenarioLibraryLoader.h"

#include <algorithm>
#include <filesystem>
#include <vector>

#include "world_model/ScenarioSerializer.h"

namespace worldmodel {

ScenarioLibrary ScenarioLibraryLoader::loadFromDirectory(
	const std::string& path) const {
	std::vector<std::filesystem::path> files;
	for (const auto& entry :
	     std::filesystem::directory_iterator(path)) {
		if (entry.path().extension() == ".toml") {
			files.push_back(entry.path());
		}
	}
	std::sort(files.begin(), files.end());

	ScenarioLibrary library;
	const ScenarioSerializer serializer;
	for (const std::filesystem::path& file : files) {
		library.add(serializer.loadFromFile(file.string()));
	}
	return library;
}

}
