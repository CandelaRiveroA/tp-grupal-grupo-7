#include "game/ViewMotion.h"

#include <algorithm>
#include <cstdlib>

#include "world/WorldConstants.h"

namespace game {

ViewMotion::ViewMotion(const world::CharacterCatalog& catalog)
	: catalog_(catalog) {}

void ViewMotion::observe(const LocalWorldView& view, int iteration) {
	if (tracks_.size() != view.entities.size()) {
		tracks_.assign(view.entities.size(), Track{});
	}

	for (std::size_t i = 0; i < view.entities.size(); ++i) {
		const ViewEntity& entity = view.entities[i];
		Track& track = tracks_[i];

		track.cadence = catalog_.dataFor(entity.kind).cadence;

		// Flanco de entrada a UseTool: la fuente acaba de reportar el
		// golpe. Guarda el tick de inicio para el reloj de animación.
		const bool usingToolNow =
			(entity.action == world::ActionKind::UseTool);
		if (usingToolNow && !track.usingTool) {
			track.toolStartIteration = iteration;
		}
		track.usingTool = usingToolNow;

		if (!track.known) {
			track.known = true;
			track.cellX = entity.cellX;
			track.cellY = entity.cellY;
			track.previousCellX = entity.cellX;
			track.previousCellY = entity.cellY;
			track.stepIteration = iteration;
			continue;
		}

		const int deltaX = entity.cellX - track.cellX;
		const int deltaY = entity.cellY - track.cellY;
		if (deltaX == 0 && deltaY == 0) {
			continue;
		}

		const bool singleStep =
			(std::abs(deltaX) + std::abs(deltaY) == 1);
		track.previousCellX = singleStep ? track.cellX : entity.cellX;
		track.previousCellY = singleStep ? track.cellY : entity.cellY;
		track.cellX = entity.cellX;
		track.cellY = entity.cellY;
		track.stepIteration = iteration;
	}
}

PixelOffset ViewMotion::offsetFor(const std::size_t index,
				  const int iteration,
				  const int cellPixelSize) const {
	if (index >= tracks_.size()) {
		return PixelOffset{0, 0};
	}
	const Track& track = tracks_[index];

	const int elapsed = std::max(0, iteration - track.stepIteration);
	const int remaining = track.cadence.stepTicks - elapsed;
	if (remaining <= 0) {
		return PixelOffset{0, 0};
	}

	const int deltaX = (track.previousCellX - track.cellX) * cellPixelSize;
	const int deltaY = (track.previousCellY - track.cellY) * cellPixelSize;
	return PixelOffset{deltaX * remaining / track.cadence.stepTicks,
			   deltaY * remaining / track.cadence.stepTicks};
}

world::PixelRect ViewMotion::drawBoxFor(const ViewEntity& entity,
					const std::size_t index,
					const int iteration,
					const int cellPixelSize) const {
	const world::ScenePixelMapper mapper(cellPixelSize);
	const world::PixelRect cell = mapper.toPixels(
		world::GridCell{entity.cellX, entity.cellY},
		world::kUnitCellSpan);
	const PixelOffset offset = offsetFor(index, iteration, cellPixelSize);
	return world::PixelRect{cell.x + offset.x, cell.y + offset.y,
				cell.width, cell.height};
}

int ViewMotion::animationTickFor(const std::size_t index,
				 const int iteration) const {
	if (index >= tracks_.size()) {
		return iteration;
	}
	return iteration - tracks_[index].stepIteration;
}

int ViewMotion::toolTickFor(const std::size_t index,
			    const int iteration) const {
	if (index >= tracks_.size()) {
		return 0;
	}
	const int elapsed = iteration - tracks_[index].toolStartIteration;
	return elapsed > 0 ? elapsed : 0;
}

bool ViewMotion::isWalking(const std::size_t index, const int iteration,
			   const bool serverMoving) const {
	return serverMoving || inTransit(index, iteration);
}

bool ViewMotion::inTransit(const std::size_t index,
			   const int iteration) const {
	if (index >= tracks_.size()) {
		return false;
	}
	const Track& track = tracks_[index];

	if (track.previousCellX == track.cellX &&
	    track.previousCellY == track.cellY) {
		return false;
	}

	const int elapsed = std::max(0, iteration - track.stepIteration);
	return elapsed < track.cadence.stepTicks;
}

}
