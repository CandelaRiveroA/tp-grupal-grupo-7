#include "game/ViewMotion.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <utility>

#include "world/WorldConstants.h"

namespace game {

ViewMotion::ViewMotion(const world::CharacterCatalog& catalog)
	: catalog_(catalog) {}

std::vector<ViewMotion::Track> ViewMotion::relinkTracks(
	const LocalWorldView& view, const int iteration) {
	std::vector<Track> relinked;
	relinked.reserve(view.entities.size());

	std::vector<bool> claimed(tracks_.size(), false);
	for (const ViewEntity& entity : view.entities) {
		std::size_t match = tracks_.size();
		for (std::size_t t = 0; t < tracks_.size(); ++t) {
			if (!claimed[t] && tracks_[t].id == entity.id) {
				match = t;
				break;
			}
		}

		Track track;
		if (match < tracks_.size()) {
			track = std::move(tracks_[match]);
			claimed[match] = true;
		}
		track.id = entity.id;
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
			// Primera vista de esta id: nace plantado en su
			// celda, sin interpolación.
			track.known = true;
			track.cellX = entity.cellX;
			track.cellY = entity.cellY;
			track.fromX = entity.cellX;
			track.fromY = entity.cellY;
			track.stepIteration = iteration;
			relinked.push_back(std::move(track));
			continue;
		}

		const int deltaX = entity.cellX - track.cellX;
		const int deltaY = entity.cellY - track.cellY;
		if (deltaX == 0 && deltaY == 0) {
			relinked.push_back(std::move(track));
			continue;
		}

		// Posición dibujada AHORA: si el paso anterior todavía está
		// en tránsito, el nuevo arranca desde ahí (el sprite no
		// salta aunque los pasos lleguen antes de la cadencia).
		double drawnX = 0.0;
		double drawnY = 0.0;
		drawnCellPosition(track, iteration, drawnX, drawnY);

		const bool singleStep = (std::abs(deltaX) + std::abs(deltaY) == 1);
		track.fromX =
			singleStep ? drawnX : static_cast<double>(entity.cellX);
		track.fromY =
			singleStep ? drawnY : static_cast<double>(entity.cellY);
		track.cellX = entity.cellX;
		track.cellY = entity.cellY;
		track.stepIteration = iteration;
		relinked.push_back(std::move(track));
	}
	return relinked;
}

void ViewMotion::observe(const LocalWorldView& view, int iteration) {
	tracks_ = relinkTracks(view, iteration);
}

PixelOffset ViewMotion::offsetFor(const Track& track, const int iteration,
				  const int cellPixelSize) const {
	const int elapsed = std::max(0, iteration - track.stepIteration);
	const int remaining = track.cadence.stepTicks - elapsed;
	if (remaining <= 0) {
		return PixelOffset{0, 0};
	}

	return PixelOffset{
		static_cast<int>((track.fromX - track.cellX) * cellPixelSize *
				 remaining / track.cadence.stepTicks),
		static_cast<int>((track.fromY - track.cellY) * cellPixelSize *
				 remaining / track.cadence.stepTicks)};
}

void ViewMotion::drawnCellPosition(const Track& track, const int iteration,
				   double& x, double& y) const {
	const int elapsed = std::max(0, iteration - track.stepIteration);
	const int remaining = track.cadence.stepTicks - elapsed;
	if (remaining <= 0) {
		x = track.cellX;
		y = track.cellY;
		return;
	}
	const double done =
		1.0 - static_cast<double>(remaining) / track.cadence.stepTicks;
	x = track.fromX + (track.cellX - track.fromX) * done;
	y = track.fromY + (track.cellY - track.fromY) * done;
}

PixelOffset ViewMotion::offsetFor(const std::size_t index,
				  const int iteration,
				  const int cellPixelSize) const {
	if (index >= tracks_.size()) {
		return PixelOffset{0, 0};
	}
	return offsetFor(tracks_[index], iteration, cellPixelSize);
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
	const Track& track = tracks_[index];
	return iteration - track.stepIteration;
}

int ViewMotion::toolTickFor(const std::size_t index,
			    const int iteration) const {
	if (index >= tracks_.size()) {
		return 0;
	}
	const int elapsed = iteration - tracks_[index].toolStartIteration;
	return elapsed > 0 ? elapsed : 0;
}

bool ViewMotion::inTransit(const Track& track, const int iteration) const {
	if (track.fromX == track.cellX && track.fromY == track.cellY) {
		return false;
	}

	const int elapsed = std::max(0, iteration - track.stepIteration);
	return elapsed < track.cadence.stepTicks;
}

bool ViewMotion::isWalking(const std::size_t index, const int iteration,
			   const bool serverMoving) const {
	if (index >= tracks_.size()) {
		return serverMoving;
	}
	return serverMoving || inTransit(tracks_[index], iteration);
}

}  // namespace game
