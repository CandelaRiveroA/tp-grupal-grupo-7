#include "game/MockAnimalSimulation.h"

#include <array>
#include <optional>
#include <utility>

#include "world/ItemKind.h"

namespace game {

namespace {

// Datos del ritmo del wander (ticks del loop). Gama corta: las gallinas
// se mueven seguido. Es dato del bot, no de presentación: el server real
// tendrá sus propios ritmos.
constexpr int kMinRestTicks = 8;
constexpr int kMaxRestTicks = 32;

// Reintento tras destino ocupado: 1 tick (el animal "vuelve a mirar").
constexpr int kRetryTicks = 1;

// Seed derivada para el stream de sonidos: decorrela de la secuencia
// base sin perder determinismo.
constexpr unsigned int kSoundSeedOffset = 0x9E3779B9u;

// Datos de la POSTURA (ticks del loop): cada animal pone un huevo cada
// `layPeriod` ticks, con layPeriod al azar en esta gama al spawn (y
// reasignado tras cada postura). Gama LARGA a propósito: la vista no se
// llena de huevos. Es dato del bot, no presentación.
constexpr int kMinLayPeriodTicks = 600;
constexpr int kMaxLayPeriodTicks = 900;

// Datos del SONIDO ambiental (ticks del loop): cada animal suelta un
// cue cada `soundPeriod` ticks, al azar en esta gama al spawn (y
// reasignado tras cada emisión). A 60 FPS la gama es ~20-45 s: con
// varios animales salen más seguido sin sincronizarse entre sí.
constexpr int kMinSoundPeriodTicks = 1200;
constexpr int kMaxSoundPeriodTicks = 2700;

// Diccionario kind -> ítem que pone (convención §4: tabla sobre switch;
// otra criatura que ponga algo = sumar una fila, ej. vaca -> leche).
constexpr std::array<std::pair<world::CreatureKind, world::ItemKind>, 2>
	kLays = {{
		{world::CreatureKind::Chicken, world::ItemKind::Egg},
		{world::CreatureKind::Cow, world::ItemKind::Milk},
	}};

// Diccionario dirección -> delta en celdas (convención §4: tabla sobre
// switch). Front es hacia abajo (coherente con MoveCommand).
constexpr std::array<DirectionDelta, 4> kDirectionDeltas = {{
	{world::Direction::Front, 0, 1},
	{world::Direction::Back, 0, -1},
	{world::Direction::Left, -1, 0},
	{world::Direction::Right, 1, 0},
}};

}

MockAnimalSimulation::MockAnimalSimulation(int gridColumns,
					   int gridRows, unsigned int seed)
	: gridColumns_(gridColumns)
	, gridRows_(gridRows)
	, random_(seed)
	, soundRandom_(seed ^ kSoundSeedOffset)
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
	animal.facing = world::Direction::Front;
	animal.moving = false;
	animal.activity = AnimalActivity::Resting;
	animal.stepIteration = 0;
	animal.restTicks = randomRestTicks();
	animal.cadence = cadence;
	// Períodos SOLO de los comportamientos que el kind tiene: dibujar
	// del RNG un período que el animal nunca usa desplazaría las
	// secuencias deterministas de los otros (los tests las pinean).
	if (producedItem(kind) != nullptr) {
		// Primera postura: layPeriod ticks de simulación después
		// del spawn (el mock spawnea antes del primer update).
		animal.layPeriod = randomLayPeriod();
		animal.nextLayIteration = animal.layPeriod;
	}
	if (cues_.cueOf(kind).has_value()) {
		animal.soundPeriod = randomSoundPeriod();
		animal.nextSoundIteration = animal.soundPeriod;
	}

	animals_.push_back(animal);
	nextId_ += 1;
	return true;
}

void MockAnimalSimulation::update(int iteration, int playerCellX,
			      int playerCellY) {
	for (AnimalRecord& animal : animals_) {
		tryProduce(animal, iteration);
		tryEmitSound(animal, iteration);

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

std::vector<ProducedItem> MockAnimalSimulation::takeProducedItems() {
	std::vector<ProducedItem> taken;
	taken.swap(producedItems_);
	return taken;
}

std::vector<ViewSoundEvent> MockAnimalSimulation::takeSoundEvents() {
	std::vector<ViewSoundEvent> taken;
	taken.swap(clucks_);
	return taken;
}

DirectionDelta MockAnimalSimulation::randomDirection() {
	std::uniform_int_distribution<int> pick(0, 3);
	return kDirectionDeltas[static_cast<std::size_t>(pick(random_))];
}

int MockAnimalSimulation::randomRestTicks() {
	std::uniform_int_distribution<int> pick(kMinRestTicks, kMaxRestTicks);
	return pick(random_);
}

int MockAnimalSimulation::randomLayPeriod() {
	std::uniform_int_distribution<int> pick(kMinLayPeriodTicks,
						kMaxLayPeriodTicks);
	return pick(random_);
}

int MockAnimalSimulation::randomSoundPeriod() {
	std::uniform_int_distribution<int> pick(kMinSoundPeriodTicks,
						kMaxSoundPeriodTicks);
	return pick(soundRandom_);
}

const world::ItemKind* MockAnimalSimulation::producedItem(
	world::CreatureKind kind) const {
	for (const auto& [laysKind, item] : kLays) {
		if (laysKind == kind) {
			return &item;
		}
	}
	return nullptr;
}

void MockAnimalSimulation::tryProduce(AnimalRecord& animal, int iteration) {
	const world::ItemKind* item = producedItem(animal.kind);
	if (item == nullptr || iteration < animal.nextLayIteration) {
		return;
	}

	// El huevo queda en la celda donde el animal está AHORA (antes de
	// moverse este tick): dato que el que llena la vista valida y
	// guarda.
	ProducedItem egg;
	egg.animalId = animal.id;
	egg.cellX = animal.cellX;
	egg.cellY = animal.cellY;
	egg.item = *item;
	producedItems_.push_back(egg);

	// El período se reasigna: la próxima postura queda espaciada (y
	// las gallinas no se sincronizan entre sí).
	animal.layPeriod = randomLayPeriod();
	animal.nextLayIteration = iteration + animal.layPeriod;
}

void MockAnimalSimulation::tryEmitSound(AnimalRecord& animal, int iteration) {
	const std::optional<CueKind> cue = cues_.cueOf(animal.kind);
	if (!cue.has_value() || iteration < animal.nextSoundIteration) {
		return;
	}

	// El sonido es ambiental: la celda no va (el id habilita
	// audio posicional mañana).
	ViewSoundEvent cluck;
	cluck.emitterId = animal.id;
	cluck.cue = *cue;
	clucks_.push_back(cluck);

	animal.soundPeriod = randomSoundPeriod();
	animal.nextSoundIteration = iteration + animal.soundPeriod;
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