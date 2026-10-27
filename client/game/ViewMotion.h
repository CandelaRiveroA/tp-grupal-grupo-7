#pragma once

#include <cstddef>
#include <vector>

#include "game/LocalWorldView.h"
#include "world/CharacterCatalog.h"
#include "world/CreatureCatalog.h"

namespace game {

struct PixelOffset {
	int x;
	int y;
};

class ViewMotion {
public:
	explicit ViewMotion(const world::CharacterCatalog& catalog);

	void observe(const LocalWorldView& view, int iteration);

	PixelOffset offsetFor(std::size_t index, int iteration,
			      int cellPixelSize) const;

	int animationTickFor(std::size_t index, int iteration) const;

	bool isWalking(std::size_t index, int iteration,
		       bool serverMoving) const;

private:
	struct Track {
		int id = 0;

		bool known = false;

		int cellX = 0;
		int cellY = 0;

		int previousCellX = 0;
		int previousCellY = 0;

		int stepIteration = 0;

		world::MoveCadence cadence{0, 0};
	};

	std::vector<Track> relinkTracks(const LocalWorldView& view,
					int iteration);

	// Estado interno de UN track: interpolación del paso en curso.
	PixelOffset offsetFor(const Track& track, int iteration,
			      int cellPixelSize) const;

	bool inTransit(const Track& track, int iteration) const;

	std::vector<Track> tracks_;

	const world::CharacterCatalog& catalog_;
};

}
