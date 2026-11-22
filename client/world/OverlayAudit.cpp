#include "world/OverlayAudit.h"

#include <set>
#include <string>

#include "world/OverlayKeys.h"

namespace world {

std::vector<ConfigError> OverlayAudit::unmatched(
	const worldmodel::World& world, const WorldOverlay& overlay) const {
	std::set<std::string> usedTiles;
	for (const auto& entry : world.maps) {
		for (const worldmodel::MapObject& object : entry.second.objects) {
			usedTiles.insert(object.tile.key());
		}
	}

	std::vector<ConfigError> findings;
	for (const auto& entry : overlay.decorations) {
		if (usedTiles.count(entry.first) == 0) {
			findings.emplace_back(
				ConfigErrorKind::UnmatchedEntry,
				std::string(kKeyOverlayDecorations) + "." +
					entry.first);
		}
	}
	for (const auto& entry : overlay.mapElements) {
		if (world.maps.count(entry.first) == 0) {
			findings.emplace_back(
				ConfigErrorKind::UnmatchedEntry,
				std::string(kKeyOverlayMaps) + "." + entry.first);
		}
	}
	return findings;
}

}
