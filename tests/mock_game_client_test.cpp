#include "gtest/gtest.h"

#include <vector>

#include "common/common_command.h"
#include "game/MockGameClient.h"
#include "world/CharacterCatalogLoader.h"
#include "world/CreatureCatalog.h"

namespace {

protocol::Command walk(int x, int y, protocol::Direction direction) {
	protocol::Command cmd;
	cmd.move = protocol::Move::Walk;
	cmd.x = static_cast<std::uint8_t>(x);
	cmd.y = static_cast<std::uint8_t>(y);
	cmd.direction = direction;
	return cmd;
}

TEST(MockGameClientTest, StartsWithLocalPlayerEntity) {
	game::MockGameClient client(10, 10, 5, 5);
	const game::LocalWorldView& view = client.worldState();
	ASSERT_EQ(view.entities.size(), 1u);
	EXPECT_EQ(view.entities[0].cellX, 5);
	EXPECT_EQ(view.entities[0].cellY, 5);
}

TEST(MockGameClientTest, WalkMovesToProposedCell) {
	game::MockGameClient client(10, 10, 5, 5);
	client.sendCommand(walk(6, 5, protocol::Direction::Right));
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_EQ(player.cellX, 6);
	EXPECT_EQ(player.cellY, 5);
	EXPECT_EQ(player.facing, protocol::Direction::Right);
	EXPECT_TRUE(player.moving);
}

TEST(MockGameClientTest, WalkToAnyCellIsAbsolute) {
	game::MockGameClient client(10, 10, 5, 5);
	client.sendCommand(walk(2, 8, protocol::Direction::Left));
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_EQ(player.cellX, 2);
	EXPECT_EQ(player.cellY, 8);
}

TEST(MockGameClientTest, ClampsTargetOutsideGrid) {
	game::MockGameClient client(10, 10, 5, 5);
	client.sendCommand(walk(99, 99, protocol::Direction::Right));
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_EQ(player.cellX, 9);
	EXPECT_EQ(player.cellY, 9);
}

TEST(MockGameClientTest, WalkToSameCellIsNotMoving) {
	game::MockGameClient client(10, 10, 5, 5);
	client.sendCommand(walk(5, 5, protocol::Direction::Front));
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_FALSE(player.moving);
	EXPECT_EQ(player.facing, protocol::Direction::Front);
}

TEST(MockGameClientTest, PickDoesNotMove) {
	game::MockGameClient client(10, 10, 5, 5);
	protocol::Command pick;
	pick.move = protocol::Move::Pick;
	pick.x = 5;
	pick.y = 5;
	pick.direction = protocol::Direction::Left;
	client.sendCommand(pick);
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_EQ(player.cellX, 5);
	EXPECT_EQ(player.cellY, 5);
	EXPECT_FALSE(player.moving);
	EXPECT_EQ(player.facing, protocol::Direction::Left);
}

TEST(MockGameClientTest, PlayerEntityCarriesItsKind) {
	game::MockGameClient client(10, 10, 5, 5);
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_EQ(player.kind, world::CreatureKind::Player);
}

TEST(MockGameClientTest, ProvidesInitialInventoryTools) {
	game::MockGameClient client(10, 10, 5, 5);
	const std::vector<game::ViewInventorySlot>& inv =
		client.worldState().inventory;
	ASSERT_EQ(inv.size(), 4u);
	EXPECT_EQ(inv[0].slotIndex, 0);
	EXPECT_EQ(inv[0].item, world::ItemKind::Shovel);
	EXPECT_EQ(inv[1].slotIndex, 1);
	EXPECT_EQ(inv[1].item, world::ItemKind::Pickaxe);
	EXPECT_EQ(inv[2].slotIndex, 2);
	EXPECT_EQ(inv[2].item, world::ItemKind::Sword);
	EXPECT_EQ(inv[3].slotIndex, 3);
	EXPECT_EQ(inv[3].item, world::ItemKind::WateringCan);
}

TEST(MockGameClientTest, InventoryStaysWhenPlayerMoves) {
	game::MockGameClient client(10, 10, 5, 5);
	client.sendCommand(walk(6, 5, protocol::Direction::Right));
	EXPECT_EQ(client.worldState().inventory.size(), 4u);
}

TEST(MockGameClientTest, WorldStateReportsGridDimensions) {
	game::MockGameClient client(10, 7, 5, 5);
	const game::LocalWorldView& view = client.worldState();
	EXPECT_EQ(view.grid.columns, 10);
	EXPECT_EQ(view.grid.rows, 7);
}

namespace {

// Catálogo mínimo (jugador + gallina) para probar el mundo con animales.
const char* const kWorldToml = R"(
[characters.player]
sheet   = "walking.png"
grid    = [16, 32]
drawn   = [24, 48]
cadence = { step = 24, frame = 12 }
  [characters.player.front]
  mirrored = false
    [characters.player.front.idle]
    loops  = false
    frames = [ [[0, 0], [6, 0]] ]
    [characters.player.front.walk]
    loops  = true
    frames = [ [[1, 0], [7, 0]], [[2, 0], [8, 0]] ]
  [characters.player.back]
  mirrored = false
    [characters.player.back.idle]
    loops  = false
    frames = [ [[0, 2], [6, 2]] ]
    [characters.player.back.walk]
    loops  = true
    frames = [ [[1, 2], [7, 2]], [[2, 2], [8, 2]] ]
  [characters.player.left]
  mirrored = true
    [characters.player.left.idle]
    loops  = false
    frames = [ [[0, 1], [6, 1]] ]
    [characters.player.left.walk]
    loops  = true
    frames = [ [[1, 1], [7, 1]], [[2, 1], [8, 1]] ]
  [characters.player.right]
  mirrored = false
    [characters.player.right.idle]
    loops  = false
    frames = [ [[0, 1], [6, 1]] ]
    [characters.player.right.walk]
    loops  = true
    frames = [ [[1, 1], [7, 1]], [[2, 1], [8, 1]] ]

[characters.chicken]
sheet   = "Chicken.png"
grid    = [16, 16]
drawn   = [32, 32]
cadence = { step = 32, frame = 8 }
  [characters.chicken.front]
  mirrored = false
    [characters.chicken.front.idle]
    loops  = false
    frames = [ [[0, 4]] ]
    [characters.chicken.front.walk]
    loops  = true
    frames = [ [[0, 0]], [[1, 0]], [[2, 0]], [[3, 0]] ]
  [characters.chicken.right]
  mirrored = false
    [characters.chicken.right.idle]
    loops  = false
    frames = [ [[2, 4]] ]
    [characters.chicken.right.walk]
    loops  = true
    frames = [ [[0, 1]], [[1, 1]], [[2, 1]], [[3, 1]] ]
  [characters.chicken.back]
  mirrored = false
    [characters.chicken.back.idle]
    loops  = false
    frames = [ [[0, 5]] ]
    [characters.chicken.back.walk]
    loops  = true
    frames = [ [[0, 2]], [[1, 2]], [[2, 2]], [[3, 2]] ]
  [characters.chicken.left]
  mirrored = false
    [characters.chicken.left.idle]
    loops  = false
    frames = [ [[2, 5]] ]
    [characters.chicken.left.walk]
    loops  = true
    frames = [ [[0, 3]], [[1, 3]], [[2, 3]], [[3, 3]] ]
)";

// El spawn de los animales es dato del mock; para que las 3 celdas caigan
// dentro, el mundo de prueba usa la grilla de la sala real (30x20).
constexpr int kRoomColumns = 30;
constexpr int kRoomRows = 20;
constexpr int kPlayerX = 15;
constexpr int kPlayerY = 10;

const world::CharacterCatalog& worldCatalog() {
	static const world::CharacterCatalog instance =
		world::CharacterCatalogLoader().loadFromString(kWorldToml);
	return instance;
}

}

TEST(MockGameClientTest, SpawnsAnimalsWhenTheCatalogHasChicken) {
	game::MockGameClient client(kRoomColumns, kRoomRows, kPlayerX,
				    kPlayerY, worldCatalog());
	const game::LocalWorldView& view = client.worldState();
	ASSERT_EQ(view.entities.size(), 4u);

	// El jugador sigue siendo la entidad 0 con id 0.
	EXPECT_EQ(view.entities[0].kind, world::CreatureKind::Player);
	EXPECT_EQ(view.entities[0].id, 0);

	for (std::size_t i = 1; i < view.entities.size(); ++i) {
		EXPECT_EQ(view.entities[i].kind,
			  world::CreatureKind::Chicken);
		EXPECT_EQ(view.entities[i].id, static_cast<int>(i));
		EXPECT_FALSE(view.entities[i].cellX == kPlayerX &&
			     view.entities[i].cellY == kPlayerY);
	}
}

TEST(MockGameClientTest, AnimalIdsAreUnique) {
	game::MockGameClient client(kRoomColumns, kRoomRows, kPlayerX,
				    kPlayerY, worldCatalog());
	const game::LocalWorldView& view = client.worldState();
	for (std::size_t a = 0; a < view.entities.size(); ++a) {
		for (std::size_t b = a + 1; b < view.entities.size(); ++b) {
			EXPECT_NE(view.entities[a].id, view.entities[b].id);
		}
	}
}

TEST(MockGameClientTest, AnimalsMoveOverUpdateTicks) {
	game::MockGameClient client(kRoomColumns, kRoomRows, kPlayerX,
				    kPlayerY, worldCatalog());

	int movedCells = 0;
	for (int it = 0; it <= 200; ++it) {
		const game::LocalWorldView& before = client.worldState();
		std::vector<int> xBefore;
		std::vector<int> yBefore;
		for (const game::ViewEntity& entity : before.entities) {
			xBefore.push_back(entity.cellX);
			yBefore.push_back(entity.cellY);
		}

		client.update(it);

		const game::LocalWorldView& after = client.worldState();
		ASSERT_EQ(after.entities.size(), before.entities.size());
		for (std::size_t i = 0; i < after.entities.size(); ++i) {
			if (after.entities[i].cellX != xBefore[i] ||
			    after.entities[i].cellY != yBefore[i]) {
				movedCells += 1;
			}
			if (after.entities[i].kind ==
			    world::CreatureKind::Player) {
				continue;
			}
			// Nadie pisa al jugador, nadie sale de la grilla.
			EXPECT_FALSE(after.entities[i].cellX == kPlayerX &&
				     after.entities[i].cellY == kPlayerY);
			EXPECT_GE(after.entities[i].cellX, 0);
			EXPECT_LT(after.entities[i].cellX, kRoomColumns);
			EXPECT_GE(after.entities[i].cellY, 0);
			EXPECT_LT(after.entities[i].cellY, kRoomRows);
		}
	}
	EXPECT_GT(movedCells, 0);
}

TEST(MockGameClientTest, AnimalActionMatchesItsMovement) {
	game::MockGameClient client(kRoomColumns, kRoomRows, kPlayerX,
				    kPlayerY, worldCatalog());
	for (int it = 0; it <= 200; ++it) {
		client.update(it);
		for (const game::ViewEntity& entity : client.worldState()
			     .entities) {
			if (entity.kind == world::CreatureKind::Player) {
				continue;
			}
			const world::ActionKind expected =
				entity.moving ? world::ActionKind::Walk
					      : world::ActionKind::Idle;
			EXPECT_EQ(entity.action, expected);
		}
	}
}

TEST(MockGameClientTest, UpdateWithoutAnimalSimulationIsANoOp) {
	game::MockGameClient client(10, 10, 5, 5);
	client.update(0);
	client.update(50);
	const game::LocalWorldView& view = client.worldState();
	ASSERT_EQ(view.entities.size(), 1u);
	EXPECT_EQ(view.entities[0].kind, world::CreatureKind::Player);
}

}
