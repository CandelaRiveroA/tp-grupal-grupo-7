#include "world/CursorCatalog.h"

namespace world {

const SpriteVisual* CursorCatalog::find(CursorKind kind) const {
	const auto it = visuals.find(kind);
	return it == visuals.end() ? nullptr : &it->second;
}

}  // namespace world
