#pragma once

#include <cstddef>
#include <vector>

#include "game/LocalWorldView.h"
#include "world/CharacterCatalog.h"
#include "world/CreatureCatalog.h"
#include "world/ScenePixelMapper.h"

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

	world::PixelRect drawBoxFor(const ViewEntity& entity, std::size_t index,
				    int iteration, int cellPixelSize) const;

	int animationTickFor(std::size_t index, int iteration) const;

	// Ticks transcurridos desde que la entidad entró en UseTool, según
	// lo observado de la fuente de estado. Reloj de la animación del
	// golpe: arranca cuando la fuente dice UseTool, no por input local.
	int toolTickFor(std::size_t index, int iteration) const;

	bool isWalking(std::size_t index, int iteration,
		       bool serverMoving) const;

private:
	struct Track {
		int id = 0;

		bool known = false;

		int cellX = 0;
		int cellY = 0;

		// Posición (fraccionaria, en celdas) desde la que arranca la
		// interpolación del paso actual. Al iniciar un paso normal es
		// la celda anterior; al re-basarse un paso a mitad de tránsito
		// es la posición dibujada en ese tick: así la continuidad del
		// sprite no depende de la cadencia.
		double fromX = 0.0;
		double fromY = 0.0;

		int stepIteration = 0;

		world::MoveCadence cadence{0, 0};

		// Trackeo del golpe (UseTool) observado de la fuente.
		bool usingTool = false;
		int toolStartIteration = 0;
	};

	std::vector<Track> relinkTracks(const LocalWorldView& view,
					int iteration);

	// Estado interno de UN track: interpolación del paso en curso.
	PixelOffset offsetFor(const Track& track, int iteration,
			      int cellPixelSize) const;

	// Posición dibujada del track en un tick, en celdas (puede ser
	// fraccionaria: el sprite va entre dos celdas durante el paso).
	void drawnCellPosition(const Track& track, int iteration,
			       double& x, double& y) const;

	bool inTransit(const Track& track, int iteration) const;

	std::vector<Track> tracks_;

	const world::CharacterCatalog& catalog_;
};

}
