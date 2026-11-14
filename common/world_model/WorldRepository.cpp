#include "world_model/WorldRepository.h"

#include <filesystem>
#include <utility>

#include "world_model/FormatError.h"
#include "world_model/SerializerIO.h"
#include "world_model/WorldSerializer.h"
#include "world_model/WorldValidator.h"

namespace worldmodel {

WorldRepository::WorldRepository(std::string directoryPath)
	: directoryPath_(std::move(directoryPath)) {}

std::vector<WorldName> WorldRepository::listWorlds() const {
	if (!std::filesystem::is_directory(directoryPath_)) {
		return {};
	}
	const WorldSerializer serializer;
	std::vector<WorldName> worlds;
	for (const std::string& path :
	     SerializerIO::tomlFilesIn(directoryPath_)) {
		try {
			const World world = serializer.loadFromFile(path);
			worlds.push_back(world.name);
		} catch (const FormatError&) {
			// Muestra el nombre del archivo para que el usuario
			// identifique cual no se puede abrir.
			worlds.push_back(
				std::filesystem::path(path).stem().string());
		}
	}
	return worlds;
}

bool WorldRepository::exists(const std::string& name) const {
	return std::filesystem::exists(filePathFor(name));
}

World WorldRepository::load(const std::string& name) const {
	if (!rules_.isValidWorldName(name)) {
		throw FormatError(FormatErrorKind::InvalidValue, "name");
	}
	return WorldSerializer().loadFromFile(filePathFor(name));
}

std::string WorldRepository::filePathFor(const std::string& name) const {
	return (std::filesystem::path(directoryPath_) / (name + ".toml"))
		.string();
}

std::vector<ValidationIssue> WorldRepository::saveIfValid(
	const World& world) const {
	const std::vector<ValidationIssue> issues =
		WorldValidator().validate(world);
	if (!issues.empty()) {
		return issues;
	}
	std::filesystem::create_directories(directoryPath_);
	WorldSerializer().saveToFile(world, filePathFor(world.name));
	return {};
}

}  // namespace worldmodel
