#include "world/EditorWorldSource.h"

#include <utility>

#include "world/MapSceneAdapter.h"
#include "world/OverlayAudit.h"

namespace world {

EditorWorldSource::EditorWorldSource(worldmodel::World world,
				     WorldOverlay overlay)
	: world_(std::move(world))
	, overlay_(std::move(overlay))
	, findings_(OverlayAudit().unmatched(world_, overlay_)) {}

const std::vector<ConfigError>& EditorWorldSource::findings() const {
	return findings_;
}

std::vector<std::string> EditorWorldSource::ids() const {
	std::vector<std::string> result;
	for (const auto& entry : world_.maps) {
		result.push_back(entry.first);
	}
	return result;
}

bool EditorWorldSource::has(const std::string& id) const {
	return world_.maps.count(id) == 1;
}

SceneDescription EditorWorldSource::load(const std::string& id) const {
	return MapSceneAdapter(overlay_).adapt(world_, id);
}

}
