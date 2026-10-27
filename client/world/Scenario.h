#pragma once

#include <string>

#include "world/SceneDescription.h"

namespace core {
class WindowConfig;
}

namespace world {

class Scenario {
public:
	Scenario(const core::WindowConfig& config, SceneDescription scene);

	int width() const;
	int height() const;

	const SceneDescription& scene() const;

	const Color& background() const;

private:
	int width_;
	int height_;
	SceneDescription scene_;
};

}
