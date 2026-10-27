#include "game/MockAnimalSimulation.h"

#include <array>
#include <utility>

namespace game {

namespace {

// Datos del ritmo del wander (ticks del loop). Gama corta: las gallinas
// se mueven seguido. Es dato del bot, no de presentación: el server real
// tendrá sus propios ritmos.
constexpr int kMinRestTicks = 8;
constexpr int kMaxRestTicks = 32;

// Reintento tras destino ocupado: 1 tick (el animal "vuelve a mirar").
constexpr int kRetryTicks = 1;

// Diccionario dirección -> delta en celdas (convención §4: tabla sobre
// switch). Front es hacia abajo (coherente con MoveCommand).
constexpr std::array<DirectionDelta, 4> kDirectionDeltas = {{
	{protocol::Direction::Front, 0, 1},
	{protocol::Direction::Back, 0, -1},
	{protocol::Direction::Left, -1, 0},
	{protocol::Direction::Right, 1, 0},
}};

}

MockAnimalSimulation::MockAnimalSimulation(int gridColumns,
					   int gridRows, unsigned int seed)
	: gridColumns_(gridColumns)
	, gridRows_(gridRows)
	, random_(seed)
	, nextId_(1) {}

bool MockAnimalSimulation::spawn(world::CreatureKind kind,
			     const world::MoveCadence& cadence,
			     world::GridCell cell, int playerCellX,
			     int playerCellY) {
	if (!canEnter(cell.column, cell.row, playerCellX, playerCellY,
		      AnimalRecord{})) {
		return false;
	}

	AnimalRecord animal;
	animal.id = nextId_;
	animal.kind = kind;
	animal.cellX = cell.column;
	animal.cellY = cell.row;
	animal.facing = protocol::Direction::Front;
	animal.moving = false;
	animal.activity = AnimalActivity::Resting;
	animal.stepIteration = 0;
	animal.restTicks = randomRestTicks();
	animal.cadence = cadence;

	animals_.push_back(animal);
	nextId_ += 1;
	return true;
}

void MockAnimalSimulation::update(int iteration, int playerCellX,
			      int playerCellY) {
	for (AnimalRecord& animal : animals_) {
		const int elapsed = iteration - animal.stepIteration;

		if (animal.activity == AnimalActivity::Stepping) {
			if (elapsed < animal.cadence.stepTicks) {
				continue;
			}
			// Cerró el paso: descansa una pausa aleatoria.
			animal.activity = AnimalActivity::Resting;
			animal.moving = false;
			animal.stepIteration = iteration;
			animal.restTicks = randomRestTicks();
			continue;
		}

		if (elapsed < animal.restTicks) {
			continue;
		}
		tryStartStep(animal, iteration, playerCellX, playerCellY);
	}
}

const std::vector<AnimalRecord>& MockAnimalSimulation::animals() const {
	return animals_;
}

DirectionDelta MockAnimalSimulation::randomDirection() {
	std::uniform_int_distribution<int> pick(0, 3);
	return kDirectionDeltas[static_cast<std::size_t>(pick(random_))];
}

int MockAnimalSimulation::randomRestTicks() {
	std::uniform_int_distribution<int> pick(kMinRestTicks, kMaxRestTicks);
	return pick(random_);
}

bool MockAnimalSimulation::canEnter(int cellX, int cellY, int playerCellX,
				int playerCellY,
				const AnimalRecord& self) const {
	const bool insideGrid = cellX >= 0 && cellX < gridColumns_ &&
				cellY >= 0 && cellY < gridRows_;
	if (!insideGrid) {
		return false;
	}
	if (cellX == playerCellX && cellY == playerCellY) {
		return false;
	}
	for (const AnimalRecord& animal : animals_) {
		if (animal.id == self.id) {
			continue;
		}
		if (animal.cellX == cellX && animal.cellY == cellY) {
			return false;
		}
	}
	return true;
}

void MockAnimalSimulation::tryStartStep(AnimalRecord& animal, int iteration,
				    int playerCellX, int playerCellY) {
	const DirectionDelta delta = randomDirection();
	const int targetX = animal.cellX + delta.deltaX;
	const int targetY = animal.cellY + delta.deltaY;

	if (!canEnter(targetX, targetY, playerCellX, playerCellY, animal)) {
		// Ocupado/fuera de grilla: reintenta en el próximo tick.
		animal.activity = AnimalActivity::Resting;
		animal.moving = false;
		animal.stepIteration = iteration;
		animal.restTicks = kRetryTicks;
		return;
	}

	// El servidor fija la celda destino YA: la vista la interpola
	// durante cadence.stepTicks (mismo modelo que el jugador).
	animal.facing = delta.direction;
	animal.cellX = targetX;
	animal.cellY = targetY;
	animal.activity = AnimalActivity::Stepping;
	animal.moving = true;
	animal.stepIteration = iteration;
}

}