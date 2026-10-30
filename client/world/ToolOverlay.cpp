#include "world/ToolOverlay.h"

namespace world {

bool ToolOverlayCatalog::has(ItemKind kind, Direction dir) const {
	const auto toolIt = tools.find(kind);
	if (toolIt == tools.end()) {
		return false;
	}
	return toolIt->second.find(dir) != toolIt->second.end();
}

const ToolOverlayAction& ToolOverlayCatalog::actionFor(ItemKind kind,
						       Direction dir) const {
	return tools.at(kind).at(dir);
}

int ToolOverlayCatalog::offsetFor(FrameAnchor anchor) const {
	const auto it = anchorOffsets.find(anchor);
	return it != anchorOffsets.end() ? it->second : 0;
}

}
