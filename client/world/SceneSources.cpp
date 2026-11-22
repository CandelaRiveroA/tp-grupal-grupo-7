#include "world/SceneSources.h"

#include <filesystem>
#include <utility>

#include "world/OverlayLoader.h"
#include "world/WorldMapLoader.h"

namespace world {

SceneSources::SceneSources(const std::string& scenesPath,
			   const std::string& worldPath,
			   const std::string& overlayPath)
	: rooms_(scenesPath) {
	if (worldPath.empty() || !std::filesystem::exists(worldPath)) {
		return;
	}
	WorldOverlay overlay;
	if (!overlayPath.empty() && std::filesystem::exists(overlayPath)) {
		overlay = OverlayLoader().loadFromFile(overlayPath);
	}
	editor_.emplace(WorldMapLoader().loadFromFile(worldPath),
			std::move(overlay));
}

std::vector<ConfigError> SceneSources::findings() const {
	if (!editor_.has_value()) {
		return {};
	}
	return editor_->findings();
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
