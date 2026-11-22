#include "world/EditorWorldSource.h"

#include <utility>

#include "world/MapSceneAdapter.h"

namespace world {

EditorWorldSource::EditorWorldSource(worldmodel::World world, int scale)
	: world_(std::move(world)), scale_(scale) {}

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
	return MapSceneAdapter(scale_).adapt(world_, id);
}

}
