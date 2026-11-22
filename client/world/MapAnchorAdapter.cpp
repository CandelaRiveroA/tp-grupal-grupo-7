#include "world/MapAnchorAdapter.h"

#include <string>

#include "world_model/WorldVocabulary.h"

namespace world {

MapAnchorAdapter::MapAnchorAdapter(const WorldOverlay& overlay)
	: overlay_(overlay) {}

void MapAnchorAdapter::add(const worldmodel::Map& map,
			   SceneDescription& scene) const {
	const worldmodel::WorldVocabulary vocabulary;
	for (const worldmodel::Marker& marker : map.markers) {
		const std::string kind = vocabulary.toText(marker.kind);
		const GridCell cell{marker.cell.column, marker.cell.row};
		scene.markers.push_back(SceneMarker{kind, cell});
		const auto npc = overlay_.markerNpcs.find(kind);
		if (npc != overlay_.markerNpcs.end()) {
			scene.npcCells[npc->second] = cell;
		}
	}
	for (const worldmodel::Portal& portal : map.portals) {
		scene.portals.push_back(ScenePortal{
			vocabulary.toText(portal.kind),
			{portal.cell.column, portal.cell.row},
			portal.targetMap,
			{portal.targetCell.column, portal.targetCell.row}});
	}
}

}
