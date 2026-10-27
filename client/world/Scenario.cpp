#include "Scenario.h"

#include "core/WindowConfig.h"
#include "world/SceneConfigLoader.h"

namespace world {

namespace {
SceneDescription loadScene(const std::string& scenesPath,
			   const std::string& roomName) {
	SceneConfigLoader loader;
	return loader.loadFromFile(scenesPath, roomName);
}
}

Scenario::Scenario(const core::WindowConfig& config,
		   const std::string& scenesPath,
		   const std::string& roomName)
	: width_(config.width())
	, height_(config.height())
	, scene_(loadScene(scenesPath, roomName)) {}

int Scenario::width() const { return width_; }

int Scenario::height() const { return height_; }

const SceneDescription& Scenario::scene() const { return scene_; }

const Color& Scenario::background() const { return scene_.floorColor; }

}
