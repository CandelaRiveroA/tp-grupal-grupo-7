#include "Scenario.h"

#include <utility>

#include "core/WindowConfig.h"

namespace world {

Scenario::Scenario(const core::WindowConfig& config, SceneDescription scene)
	: width_(config.width())
	, height_(config.height())
	, scene_(std::move(scene)) {}

int Scenario::width() const { return width_; }

int Scenario::height() const { return height_; }

const SceneDescription& Scenario::scene() const { return scene_; }

const Color& Scenario::background() const { return scene_.floorColor; }

}
