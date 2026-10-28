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

[characters.cow]
sheet   = "Cow.png"
grid    = [32, 32]
drawn   = [64, 64]
cadence = { step = 32, frame = 32 }
  [characters.cow.front]
  mirrored = false
    [characters.cow.front.idle]
    loops  = false
    frames = [ [[0, 1]] ]
    [characters.cow.front.walk]
    loops  = true
    frames = [ [[0, 1]] ]
  [characters.cow.right]
  mirrored = false
    [characters.cow.right.idle]
    loops  = false
    frames = [ [[0, 1]] ]
    [characters.cow.right.walk]
    loops  = true
    frames = [ [[0, 1]] ]
  [characters.cow.back]
  mirrored = false
    [characters.cow.back.idle]
    loops  = false
    frames = [ [[0, 2]] ]
    [characters.cow.back.walk]
    loops  = true
    frames = [ [[0, 2]] ]
  [characters.cow.left]
  mirrored = false
    [characters.cow.left.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.cow.left.walk]
    loops  = true
    frames = [ [[0, 0]] ]
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

TEST(MockGameClientTest, SpawnsAnimalsWhenTheCatalogHasThem) {
	game::MockGameClient client(kRoomColumns, kRoomRows, kPlayerX,
				    kPlayerY, worldCatalog());
	const game::LocalWorldView& view = client.worldState();
	// 3 gallinas + 3 vacas + jugador, en orden de spawn.
	ASSERT_EQ(view.entities.size(), 7u);

	// El jugador sigue siendo la entidad 0 con id 0.
	EXPECT_EQ(view.entities[0].kind, world::CreatureKind::Player);
	EXPECT_EQ(view.entities[0].id, 0);

	// El orden del estado replica el orden de spawn: gallinas y luego
	// vacas (el orden de la tabla de bindings del mock).
	for (std::size_t i = 1; i <= 3; ++i) {
		EXPECT_EQ(view.entities[i].kind,
			  world::CreatureKind::Chicken);
	}
	for (std::size_t i = 4; i < view.entities.size(); ++i) {
		EXPECT_EQ(view.entities[i].kind, world::CreatureKind::Cow);
	}
	for (std::size_t i = 1; i < view.entities.size(); ++i) {
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

// --- Huevos: la gallina pone, el ítem aparece en el suelo ---------------

namespace {

// Camina el mundo N ticks acumulando los ítems del suelo visibles en
// cada frame (el último estado manda).
std::vector<game::ViewGroundItem> groundItemsAfter(
	game::MockGameClient& client, int lastIteration) {
	std::vector<game::ViewGroundItem> last;
	for (int it = 0; it <= lastIteration; ++it) {
		client.update(it);
		last = client.worldState().groundItems;
	}
	return last;
}

}

TEST(MockGameClientTest, ChickenLaysEggsOnTheGround) {
	game::MockGameClient client(kRoomColumns, kRoomRows, kPlayerX,
				    kPlayerY, worldCatalog());

	// El período de postura es largo (gama en el .cpp de la
	// simulación) pero acotado: con esta seed la primera postura cae
	// bien dentro de la caminata.
	std::vector<game::ViewGroundItem> eggs =
		groundItemsAfter(client, 3000);
	ASSERT_FALSE(eggs.empty());

	for (const game::ViewGroundItem& egg : eggs) {
		EXPECT_EQ(egg.item, world::ItemKind::Egg);
		EXPECT_GE(egg.cellX, 0);
		EXPECT_LT(egg.cellX, kRoomColumns);
		EXPECT_GE(egg.cellY, 0);
		EXPECT_LT(egg.cellY, kRoomRows);
		// Nadie pone un huevo en la celda del jugador.
		EXPECT_FALSE(egg.cellX == kPlayerX && egg.cellY == kPlayerY);
	}
}

TEST(MockGameClientTest, GroundEggCountNeverExceedsTheCap) {
	// Sin recogida, el tope de ítems en el suelo es lo que evita que
	// la vista se llene de huevos (regla silenciosa del "server
	// falso", como el límite de slots del inventario).
	game::MockGameClient client(kRoomColumns, kRoomRows, kPlayerX,
				    kPlayerY, worldCatalog());
	const std::vector<game::ViewGroundItem> eggs =
		groundItemsAfter(client, 8000);
	EXPECT_LE(eggs.size(), 3u);
}

TEST(MockGameClientTest, GroundItemIdsAreStableBetweenFrames) {
	game::MockGameClient client(kRoomColumns, kRoomRows, kPlayerX,
				    kPlayerY, worldCatalog());
	const std::vector<game::ViewGroundItem> eggs =
		groundItemsAfter(client, 3000);
	ASSERT_FALSE(eggs.empty());

	std::vector<int> ids;
	for (const game::ViewGroundItem& egg : eggs) {
		ids.push_back(egg.id);
	}

	// 200 ticks más: los ids de antes SIGUEN (los ítems del suelo no
	// desaparecen) y siguen siendo únicos.
	const std::vector<game::ViewGroundItem> later =
		groundItemsAfter(client, 3200);
	for (const int id : ids) {
		bool found = false;
		for (const game::ViewGroundItem& egg : later) {
			found = found || egg.id == id;
		}
		EXPECT_TRUE(found) << "id " << id << " desapareció";
	}
	for (std::size_t a = 0; a < later.size(); ++a) {
		for (std::size_t b = a + 1; b < later.size(); ++b) {
			EXPECT_NE(later[a].id, later[b].id);
		}
	}
}

TEST(MockGameClientTest, TwoGroundEggsNeverShareACell) {
	game::MockGameClient client(kRoomColumns, kRoomRows, kPlayerX,
				    kPlayerY, worldCatalog());
	for (int it = 0; it <= 8000; ++it) {
		client.update(it);
		const std::vector<game::ViewGroundItem>& eggs =
			client.worldState().groundItems;
		for (std::size_t a = 0; a < eggs.size(); ++a) {
			for (std::size_t b = a + 1; b < eggs.size(); ++b) {
				EXPECT_FALSE(eggs[a].cellX == eggs[b].cellX &&
					     eggs[a].cellY == eggs[b].cellY)
					<< "huevos " << eggs[a].id << " y "
					<< eggs[b].id << " comparten celda";
			}
		}
	}
}

TEST(MockGameClientTest, WorldWithoutAnimalSimulationHasNoGroundItems) {
	game::MockGameClient client(10, 10, 5, 5);
	const std::vector<game::ViewGroundItem> eggs =
		groundItemsAfter(client, 3000);
	EXPECT_TRUE(eggs.empty());
}

// --- Cacareos: eventos de sonido per-tick en la vista -------------------

namespace {

// Camina el mundo hasta ver el primer cacareo en la vista; devuelve el
// tick en que apareció (o -1 si no apareció).
int firstCluckTick(game::MockGameClient& client, int limit) {
	for (int it = 0; it <= limit; ++it) {
		client.update(it);
		if (!client.worldState().soundEvents.empty()) {
			return it;
		}
	}
	return -1;
}

}

TEST(MockGameClientTest, ChickenClucksArriveAsSoundEvents) {
	game::MockGameClient client(kRoomColumns, kRoomRows, kPlayerX,
				    kPlayerY, worldCatalog());

	// El período del cacareo es largo pero acotado: con esta seed
	// algún cacareo cae dentro de la ventana.
	const int cluckTick = firstCluckTick(client, 3000);
	ASSERT_GE(cluckTick, 0)
		<< "con esta seed alguna gallina debería cacarear";

	const game::LocalWorldView& view = client.worldState();
	const game::ViewSoundEvent& cluck = view.soundEvents.front();
	// El evento referencia a una ENTIDAD visible (id estable).
	EXPECT_EQ(cluck.cue, game::CueKind::ChickenCluck);
	bool emitterVisible = false;
	for (const game::ViewEntity& entity : view.entities) {
		emitterVisible = emitterVisible ||
				 entity.id == cluck.emitterId;
	}
	EXPECT_TRUE(emitterVisible)
		<< "el cacareo referencia a una gallina de la vista";
}

TEST(MockGameClientTest, SoundEventsLastOneTick) {
	// Los eventos de sonido son PER-TICK: al tick siguiente del
	// cacareo (sin otro cacareo: los períodos son de ~1200+ ticks)
	// la vista ya no lleva ninguno.
	game::MockGameClient client(kRoomColumns, kRoomRows, kPlayerX,
				    kPlayerY, worldCatalog());
	const int cluckTick = firstCluckTick(client, 3000);
	ASSERT_GE(cluckTick, 0);

	client.update(cluckTick + 1);
	EXPECT_TRUE(client.worldState().soundEvents.empty());
}

TEST(MockGameClientTest, WorldWithoutAnimalSimulationHasNoSoundEvents) {
	game::MockGameClient client(10, 10, 5, 5);
	EXPECT_TRUE(firstCluckTick(client, 100) == -1);
	EXPECT_TRUE(client.worldState().soundEvents.empty());
}

}
