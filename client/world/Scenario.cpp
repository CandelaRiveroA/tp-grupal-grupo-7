#include "Scenario.h"

#include "core/WindowConfig.h"

namespace world {

Scenario::Scenario(const core::WindowConfig& config, const Player& player)
	: width_(config.width())
	, height_(config.height())
	, background_{60, 60, 60, 255}
	, player_(&player) {}

int Scenario::width() const { return width_; }

int Scenario::height() const { return height_; }

const Color& Scenario::background() const { return background_; }

const Player& Scenario::player() const { return *player_; }

}
