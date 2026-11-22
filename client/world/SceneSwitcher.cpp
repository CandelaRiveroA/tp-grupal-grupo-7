#include "world/SceneSwitcher.h"

#include <algorithm>
#include <utility>

#include "world/ConfigError.h"
#include "world/SceneKeys.h"

namespace world {

SceneSwitcher::SceneSwitcher(std::vector<const SceneSource*> sources,
			     std::string initialId,
			     SceneDescription initialScene)
	: sources_(std::move(sources))
	, id_(std::move(initialId))
	, scene_(std::move(initialScene)) {}

const SceneDescription& SceneSwitcher::scene() const { return scene_; }

const std::string& SceneSwitcher::id() const { return id_; }

void SceneSwitcher::switchTo(const std::string& id) {
	for (const SceneSource* source : sources_) {
		if (!source->has(id)) {
			continue;
		}
		scene_ = source->load(id);
		id_ = id;
		return;
	}
	throw ConfigError(ConfigErrorKind::MissingStructure,
			  std::string(kKeyRooms) + "." + id);
}

std::vector<std::string> SceneSwitcher::allIds() const {
	std::vector<std::string> all;
	for (const SceneSource* source : sources_) {
		for (const std::string& id : source->ids()) {
			if (std::find(all.begin(), all.end(), id) == all.end()) {
				all.push_back(id);
			}
		}
	}
	return all;
}

std::string SceneSwitcher::nextId() const {
	const std::vector<std::string> all = allIds();
	if (all.empty()) {
		return id_;
	}
	const auto found = std::find(all.begin(), all.end(), id_);
	if (found == all.end() || found + 1 == all.end()) {
		return all.front();
	}
	return *(found + 1);
}

}
