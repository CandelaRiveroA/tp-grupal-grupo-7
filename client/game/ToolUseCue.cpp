#include "game/ToolUseCue.h"

namespace game {

std::optional<world::ItemKind> ToolUseCue::update(
	const std::optional<world::ItemKind>& activeTool) {
	if (!activeTool.has_value()) {
		using_ = false;
		return std::nullopt;
	}
	if (using_) {
		return std::nullopt;
	}
	using_ = true;
	return activeTool;
}

}  // namespace game
