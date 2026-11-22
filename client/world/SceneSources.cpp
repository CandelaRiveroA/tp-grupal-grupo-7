#include "world/SceneSources.h"

#include <filesystem>

#include "world/WorldMapLoader.h"

namespace world {

SceneSources::SceneSources(const std::string& scenesPath,
			   const std::string& worldPath, int worldScale)
	: rooms_(scenesPath) {
	if (worldPath.empty() || !std::filesystem::exists(worldPath)) {
		return;
	}
	editor_.emplace(WorldMapLoader().loadFromFile(worldPath), worldScale);
}

std::vector<const SceneSource*> SceneSources::byPriority() const {
	std::vector<const SceneSource*> sources;
	if (editor_.has_value()) {
		sources.push_back(&*editor_);
	}
	sources.push_back(&rooms_);
	return sources;
}

}
