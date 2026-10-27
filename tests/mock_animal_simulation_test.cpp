#include "gtest/gtest.h"

#include <cstdlib>
#include <optional>
#include <utility>
#include <vector>

#include "game/MockAnimalSimulation.h"
#include "world/CreatureCatalog.h"
#include "world/Grid.h"
#include "world/ItemCatalog.h"

namespace {

using game::AnimalActivity;
using game::AnimalRecord;
using game::MockAnimalSimulation;

constexpr int kColumns = 10;
constexpr int kRows = 10;
constexpr unsigned int kSeed = 1;
constexpr int kStepTicks = 24;

world::MoveCadence cadence() {
	return world::MoveCadence{kStepTicks, 12};
}

MockAnimalSimulation makeSimulation(unsigned int seed = kSeed) {
	return MockAnimalSimulation(kColumns, kRows, seed);
}

bool spawnAt(MockAnimalSimulation& simulation, world::GridCell cell) {
	return simulation.spawn(world::CreatureKind::Chicken, cadence(), cell,
				5, 5);
}

const AnimalRecord& onlyAnimal(const MockAnimalSimulation& simulation) {
	return simulation.animals().at(0);
}

TEST(MockAnimalSimulationTest, SpawnsOneAnimalPerFreeCell) {
	MockAnimalSimulation simulation = makeSimulation();
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{3, 3}));
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{6, 4}));
	EXPECT_EQ(simulation.animals().size(), 2u);
}

TEST(MockAnimalSimulationTest, AssignsStableSequentialIds) {
	MockAnimalSimulation simulation = makeSimulation();
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{3, 3}));
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{6, 4}));
	EXPECT_EQ(simulation.animals()[0].id, 1);
	EXPECT_EQ(simulation.animals()[1].id, 2);
}

TEST(MockAnimalSimulationTest, RejectsSpawnOutsideTheGrid) {
	MockAnimalSimulation simulation = makeSimulation();
	EXPECT_FALSE(spawnAt(simulation, world::GridCell{-1, 3}));
	EXPECT_FALSE(spawnAt(simulation, world::GridCell{10, 3}));
	EXPECT_FALSE(spawnAt(simulation, world::GridCell{3, -1}));
	EXPECT_FALSE(spawnAt(simulation, world::GridCell{3, 10}));
	EXPECT_TRUE(simulation.animals().empty());
}

TEST(MockAnimalSimulationTest, RejectsSpawnOnAnOccupiedCell) {
	MockAnimalSimulation simulation = makeSimulation();
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{3, 3}));
	EXPECT_FALSE(spawnAt(simulation, world::GridCell{3, 3}));
	EXPECT_EQ(simulation.animals().size(), 1u);
}

TEST(MockAnimalSimulationTest, RejectsSpawnOnThePlayerCell) {
	MockAnimalSimulation simulation = makeSimulation();
	const bool spawned = simulation.spawn(
		world::CreatureKind::Chicken, cadence(), world::GridCell{5, 5},
		5, 5);
	EXPECT_FALSE(spawned);
	EXPECT_TRUE(simulation.animals().empty());
}

TEST(MockAnimalSimulationTest, SpawnsRestingInItsCell) {
	MockAnimalSimulation simulation = makeSimulation();
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{3, 3}));
	EXPECT_EQ(onlyAnimal(simulation).activity, AnimalActivity::Resting);
	EXPECT_FALSE(onlyAnimal(simulation).moving);
	EXPECT_EQ(onlyAnimal(simulation).cellX, 3);
	EXPECT_EQ(onlyAnimal(simulation).cellY, 3);
}

TEST(MockAnimalSimulationTest, RestingAnimalDoesNotMoveBeforeItsRestEnds) {
	MockAnimalSimulation simulation = makeSimulation();
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{3, 3}));
	for (int it = 0; it <= 2; ++it) {
		simulation.update(it, 5, 5);
		const AnimalRecord& animal = onlyAnimal(simulation);
		if (animal.activity == AnimalActivity::Resting) {
			EXPECT_EQ(animal.cellX, 3);
			EXPECT_EQ(animal.cellY, 3);
			EXPECT_FALSE(animal.moving);
		}
	}
}

TEST(MockAnimalSimulationTest, SteppingAnimalMovesOneCellAndThenRests) {
	MockAnimalSimulation simulation = makeSimulation();
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{3, 3}));

	// Scan determinista con seed fija: captura el tick en que arranca
	// el primer paso y verifica la ventana completa del paso.
	std::optional<int> stepStart;
	std::optional<protocol::Direction> stepFacing;
	for (int it = 0; it <= 300 && !stepStart.has_value(); ++it) {
		simulation.update(it, 5, 5);
		if (onlyAnimal(simulation).moving) {
			stepStart = it;
			stepFacing = onlyAnimal(simulation).facing;
		}
	}
	ASSERT_TRUE(stepStart.has_value())
		<< "con esta seed el animal debería arrancar un paso";

	for (int it = *stepStart + 1; it < *stepStart + kStepTicks; ++it) {
		simulation.update(it, 5, 5);
		const AnimalRecord& animal = onlyAnimal(simulation);
		EXPECT_EQ(animal.activity, AnimalActivity::Stepping)
			<< "it=" << it;
		EXPECT_TRUE(animal.moving);
	}

	simulation.update(*stepStart + kStepTicks, 5, 5);
	const AnimalRecord& animal = onlyAnimal(simulation);
	EXPECT_EQ(animal.activity, AnimalActivity::Resting);
	EXPECT_FALSE(animal.moving);
	EXPECT_NE(stepFacing, std::nullopt);
}

TEST(MockAnimalSimulationTest, StepIsExactlyOneCellTowardsFacing) {
	MockAnimalSimulation simulation = makeSimulation();
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{3, 3}));

	// En CADA arranque de paso (moving pasa de false a true), la
	// celda fijada como destino queda exactamente a 1 celda de la
	// anterior, y el facing apunta hacia ese delta.
	bool wasMoving = false;
	int cellXBefore = 3;
	int cellYBefore = 3;
	std::size_t stepStarts = 0;
	for (int it = 0; it <= 300; ++it) {
		simulation.update(it, 5, 5);
		const AnimalRecord& animal = onlyAnimal(simulation);
		if (animal.moving && !wasMoving) {
			const int deltaX = animal.cellX - cellXBefore;
			const int deltaY = animal.cellY - cellYBefore;
			EXPECT_EQ(std::abs(deltaX) + std::abs(deltaY), 1)
				<< "it=" << it;
			const protocol::Direction expectedFacing =
				deltaX == 1 ? protocol::Direction::Right
				: deltaX == -1 ? protocol::Direction::Left
				: deltaY == 1 ? protocol::Direction::Front
					      : protocol::Direction::Back;
			EXPECT_EQ(animal.facing, expectedFacing)
				<< "it=" << it;
			stepStarts += 1;
		}
		wasMoving = animal.moving;
		cellXBefore = animal.cellX;
		cellYBefore = animal.cellY;
	}
	EXPECT_GT(stepStarts, 0u) << "la caminata debería arrancar pasos";
}

TEST(MockAnimalSimulationTest, NeverStepsOntoThePlayerOrAnotherAnimal) {
	MockAnimalSimulation simulation = makeSimulation();
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{3, 3}));
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{6, 6}));

	constexpr int kPlayerX = 5;
	constexpr int kPlayerY = 5;
	for (int it = 0; it <= 400; ++it) {
		simulation.update(it, kPlayerX, kPlayerY);
		for (const AnimalRecord& animal : simulation.animals()) {
			EXPECT_FALSE(animal.cellX == kPlayerX &&
				     animal.cellY == kPlayerY)
				<< "animal " << animal.id << " pisa al jugador";
			EXPECT_GE(animal.cellX, 0);
			EXPECT_LT(animal.cellX, kColumns);
			EXPECT_GE(animal.cellY, 0);
			EXPECT_LT(animal.cellY, kRows);
		}
		for (const AnimalRecord& a : simulation.animals()) {
			for (const AnimalRecord& b : simulation.animals()) {
				if (a.id == b.id) {
					continue;
				}
				EXPECT_FALSE(a.cellX == b.cellX &&
					     a.cellY == b.cellY)
					<< a.id << " y " << b.id
					<< " comparten celda";
			}
		}
	}
}

TEST(MockAnimalSimulationTest, SameSeedProducesTheSameWander) {
	MockAnimalSimulation first = makeSimulation(kSeed);
	MockAnimalSimulation second = makeSimulation(kSeed);
	EXPECT_TRUE(spawnAt(first, world::GridCell{3, 3}));
	EXPECT_TRUE(spawnAt(second, world::GridCell{3, 3}));

	for (int it = 0; it <= 300; ++it) {
		first.update(it, 5, 5);
		second.update(it, 5, 5);
		ASSERT_EQ(first.animals().size(), second.animals().size());
		EXPECT_EQ(onlyAnimal(first).cellX, onlyAnimal(second).cellX);
		EXPECT_EQ(onlyAnimal(first).cellY, onlyAnimal(second).cellY);
		EXPECT_EQ(onlyAnimal(first).moving, onlyAnimal(second).moving);
	}
}

TEST(MockAnimalSimulationTest, FacingIsAlwaysAValidDirection) {
	MockAnimalSimulation simulation = makeSimulation();
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{3, 3}));
	for (int it = 0; it <= 300; ++it) {
		simulation.update(it, 5, 5);
		const protocol::Direction facing = onlyAnimal(simulation).facing;
		const bool valid =
			facing == protocol::Direction::Front ||
			facing == protocol::Direction::Back ||
			facing == protocol::Direction::Left ||
			facing == protocol::Direction::Right;
		EXPECT_TRUE(valid);
	}
}

// --- Postura de huevos --------------------------------------------------

// Recolecta lo que el animal puso durante una caminata de ticks,
// drenando la simulación en cada tick.
std::vector<game::LaidEgg> laysOver(MockAnimalSimulation& simulation,
				    int lastIteration) {
	std::vector<game::LaidEgg> lays;
	for (int it = 0; it <= lastIteration; ++it) {
		simulation.update(it, 5, 5);
		for (game::LaidEgg& egg : simulation.takeLaidEggs()) {
			lays.push_back(std::move(egg));
		}
	}
	return lays;
}

TEST(MockAnimalSimulationTest, DoesNotLayRightAfterSpawning) {
	// El período de postura es LARGO (gama en el .cpp de la
	// simulación): la vista no se llena de huevos. Arrancar la
	// caminata y drenar no produce posturas inmediatas.
	MockAnimalSimulation simulation = makeSimulation();
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{3, 3}));
	EXPECT_TRUE(laysOver(simulation, 100).empty());
}

TEST(MockAnimalSimulationTest, LaysAtItsOwnCellWhenItsPeriodEnds) {
	MockAnimalSimulation simulation = makeSimulation();
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{3, 3}));

	std::optional<game::LaidEgg> firstLay;
	int cellXBefore = 3;
	int cellYBefore = 3;
	for (int it = 0; it <= 2000 && !firstLay.has_value(); ++it) {
		cellXBefore = onlyAnimal(simulation).cellX;
		cellYBefore = onlyAnimal(simulation).cellY;
		simulation.update(it, 5, 5);
		std::vector<game::LaidEgg> lays = simulation.takeLaidEggs();
		if (!lays.empty()) {
			firstLay = std::move(lays.front());
		}
	}
	ASSERT_TRUE(firstLay.has_value())
		<< "con esta seed la gallina debería poner dentro del "
		   "período escaneado";

	// El huevo queda en la celda DONDE ESTABA la gallina al ponerlo,
	// es del ítem que la tabla de postura define para el kind, y
	// referencia a la gallina que lo puso.
	EXPECT_EQ(firstLay->cellX, cellXBefore);
	EXPECT_EQ(firstLay->cellY, cellYBefore);
	EXPECT_EQ(firstLay->item, world::ItemKind::Egg);
	EXPECT_EQ(firstLay->animalId, onlyAnimal(simulation).id);
}

TEST(MockAnimalSimulationTest, TakeLaidEggsDrains) {
	MockAnimalSimulation simulation = makeSimulation();
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{3, 3}));
	EXPECT_FALSE(laysOver(simulation, 2000).empty());
	// Drenar dos veces: la segunda no repite lo ya entregado.
	EXPECT_TRUE(simulation.takeLaidEggs().empty());
}

TEST(MockAnimalSimulationTest, LaysAgainAfterRescheduling) {
	MockAnimalSimulation simulation = makeSimulation();
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{3, 3}));

	// La caminata completa produce al menos DOS posturas: tras poner,
	// el período se reasigna y la gallina vuelve a poner más tarde.
	std::vector<game::LaidEgg> lays = laysOver(simulation, 4000);
	EXPECT_GE(lays.size(), 2u);
}

TEST(MockAnimalSimulationTest, SameSeedProducesTheSameLays) {
	MockAnimalSimulation first = makeSimulation(kSeed);
	MockAnimalSimulation second = makeSimulation(kSeed);
	EXPECT_TRUE(spawnAt(first, world::GridCell{3, 3}));
	EXPECT_TRUE(spawnAt(second, world::GridCell{3, 3}));

	const std::vector<game::LaidEgg> firstLays =
		laysOver(first, 2000);
	const std::vector<game::LaidEgg> secondLays =
		laysOver(second, 2000);
	ASSERT_EQ(firstLays.size(), secondLays.size());
	for (std::size_t i = 0; i < firstLays.size(); ++i) {
		EXPECT_EQ(firstLays[i].cellX, secondLays[i].cellX);
		EXPECT_EQ(firstLays[i].cellY, secondLays[i].cellY);
		EXPECT_EQ(firstLays[i].item, secondLays[i].item);
	}
}

TEST(MockAnimalSimulationTest, KindNotInTheLayTableDoesNotLay) {
	// La postura es una TABLA kind -> ítem: un kind que no está no
	// pone nada (mañana: la vaca entra sumando una fila).
	MockAnimalSimulation simulation = makeSimulation();
	const bool spawned = simulation.spawn(
		world::CreatureKind::Player, cadence(), world::GridCell{3, 3},
		5, 5);
	ASSERT_TRUE(spawned);
	EXPECT_TRUE(laysOver(simulation, 2000).empty());
}

// --- Cacareos -----------------------------------------------------------

// Recolecta lo que los animales soltaron durante una caminata de
// ticks, drenando la simulación en cada tick.
std::vector<game::ViewSoundEvent> clucksOver(
	MockAnimalSimulation& simulation, int lastIteration) {
	std::vector<game::ViewSoundEvent> clucks;
	for (int it = 0; it <= lastIteration; ++it) {
		simulation.update(it, 5, 5);
		for (game::ViewSoundEvent& cluck : simulation.takeClucks()) {
			clucks.push_back(std::move(cluck));
		}
	}
	return clucks;
}

TEST(MockAnimalSimulationTest, DoesNotCluckRightAfterSpawning) {
	// El período del cacareo es LARGO (gama en el .cpp): drenar
	// 100 ticks no produce cacareos inmediatos.
	MockAnimalSimulation simulation = makeSimulation();
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{3, 3}));
	EXPECT_TRUE(clucksOver(simulation, 100).empty());
}

TEST(MockAnimalSimulationTest, ClucksWhenItsPeriodEnds) {
	MockAnimalSimulation simulation = makeSimulation();
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{3, 3}));

	std::optional<game::ViewSoundEvent> firstCluck;
	for (int it = 0; it <= 3000 && !firstCluck.has_value(); ++it) {
		simulation.update(it, 5, 5);
		std::vector<game::ViewSoundEvent> clucks =
			simulation.takeClucks();
		if (!clucks.empty()) {
			firstCluck = std::move(clucks.front());
		}
	}
	ASSERT_TRUE(firstCluck.has_value())
		<< "con esta seed la gallina debería cacarear dentro de "
		   "la ventana escaneada";

	// El evento referencia a la gallina que lo soltó (id estable,
	// el de la vista) y trae el cue que el SoundCueCatalog asocia
	// al kind.
	EXPECT_EQ(firstCluck->emitterId, onlyAnimal(simulation).id);
	EXPECT_EQ(firstCluck->cue, game::CueKind::ChickenCluck);
}

TEST(MockAnimalSimulationTest, TakeClucksDrains) {
	MockAnimalSimulation simulation = makeSimulation();
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{3, 3}));
	EXPECT_FALSE(clucksOver(simulation, 3000).empty());
	// Drenar dos veces: la segunda no repite lo ya entregado.
	EXPECT_TRUE(simulation.takeClucks().empty());
}

TEST(MockAnimalSimulationTest, ClucksAgainAfterRescheduling) {
	MockAnimalSimulation simulation = makeSimulation();
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{3, 3}));

	// La caminata completa produce al menos DOS cacareos: tras
	// soltar uno, el período se reasigna y vuelve a cacarear más
	// tarde.
	EXPECT_GE(clucksOver(simulation, 9000).size(), 2u);
}

TEST(MockAnimalSimulationTest, CluckGapsStayInTheAmbientRange) {
	// Cada cacareo queda programado a la gama ambiental del anterior
	// (~20-45 s a 60 FPS): ni silencio infinito ni metralla de
	// cacareos.
	MockAnimalSimulation simulation = makeSimulation();
	EXPECT_TRUE(spawnAt(simulation, world::GridCell{3, 3}));

	constexpr int kMinGap = 1200;
	constexpr int kMaxGap = 2700;
	int lastCluckIteration = 0;
	std::size_t clucks = 0;
	for (int it = 0; it <= 9000; ++it) {
		simulation.update(it, 5, 5);
		if (!simulation.takeClucks().empty()) {
			const int gap = it - lastCluckIteration;
			EXPECT_GE(gap, kMinGap) << "it=" << it;
			EXPECT_LE(gap, kMaxGap) << "it=" << it;
			lastCluckIteration = it;
			++clucks;
		}
	}
	EXPECT_GE(clucks, 2u) << "la caminata debería producir cacareos";
}

TEST(MockAnimalSimulationTest, SameSeedProducesTheSameClucks) {
	MockAnimalSimulation first = makeSimulation(kSeed);
	MockAnimalSimulation second = makeSimulation(kSeed);
	EXPECT_TRUE(spawnAt(first, world::GridCell{3, 3}));
	EXPECT_TRUE(spawnAt(second, world::GridCell{3, 3}));

	const std::vector<game::ViewSoundEvent> firstClucks =
		clucksOver(first, 3000);
	const std::vector<game::ViewSoundEvent> secondClucks =
		clucksOver(second, 3000);
	ASSERT_EQ(firstClucks.size(), secondClucks.size());
	for (std::size_t i = 0; i < firstClucks.size(); ++i) {
		EXPECT_EQ(firstClucks[i].emitterId,
			  secondClucks[i].emitterId);
		EXPECT_EQ(firstClucks[i].cue, secondClucks[i].cue);
	}
}

TEST(MockAnimalSimulationTest, KindNotInTheCluckTableDoesNotCluck) {
	MockAnimalSimulation simulation = makeSimulation();
	const bool spawned = simulation.spawn(
		world::CreatureKind::Player, cadence(), world::GridCell{3, 3},
		5, 5);
	ASSERT_TRUE(spawned);
	EXPECT_TRUE(clucksOver(simulation, 3000).empty());
}

}
