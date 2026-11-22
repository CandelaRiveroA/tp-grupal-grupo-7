#include "world/SceneSwitcher.h"

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

}
