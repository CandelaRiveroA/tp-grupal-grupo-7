#include "gtest/gtest.h"

#include <cstdlib>
#include <optional>

#include "game/MockAnimalSimulation.h"
#include "world/CreatureCatalog.h"
#include "world/Grid.h"

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

}
