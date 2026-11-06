#include "world/ToolOverlay.h"

namespace world {

const ToolOverlayAction* ToolOverlayCatalog::find(ItemKind kind,
						  Direction dir) const {
	const auto toolIt = tools.find(kind);
	if (toolIt == tools.end()) {
		return nullptr;
	}
	const auto it = toolIt->second.find(dir);
	return it == toolIt->second.end() ? nullptr : &it->second;
}

const HeldContentOverlay* ToolOverlayCatalog::findHeldContent(
	const Direction dir) const {
	const auto it = heldContent.find(dir);
	return it == heldContent.end() ? nullptr : &it->second;
}

int ToolOverlayCatalog::offsetFor(FrameAnchor anchor) const {
	const auto it = anchorOffsets.find(anchor);
	return it != anchorOffsets.end() ? it->second : 0;
}

}
