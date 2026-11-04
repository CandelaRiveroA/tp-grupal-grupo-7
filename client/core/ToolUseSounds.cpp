#include "core/ToolUseSounds.h"

namespace core {

ToolUseSounds::ToolUseSounds()
	: sounds_{
		  {world::ItemKind::Shovel, sdl::SoundId::ShovelDig},
		  {world::ItemKind::WateringCan, sdl::SoundId::WateringCanPour},
	  } {}

std::optional<sdl::SoundId> ToolUseSounds::update(
	const std::optional<world::ItemKind>& activeTool) {
	const std::optional<world::ItemKind> started = cue_.update(activeTool);
	if (!started.has_value()) {
		return std::nullopt;
	}
	const auto sound = sounds_.find(*started);
	if (sound == sounds_.end()) {
		return std::nullopt;
	}
	return sound->second;
}

}  // namespace core
