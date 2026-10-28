#include "gtest/gtest.h"

#include <optional>
#include <string>
#include <vector>

#include "common/common_command.h"
#include "game/MockGameClient.h"
#include "world/CreatureCatalog.h"

namespace {

// Duración del golpe (en consultas de estado) que el mock simula.
constexpr int kToolTicks = 30;

protocol::Command walk(int x, int y, protocol::Direction direction) {
	protocol::Command cmd;
	cmd.move = protocol::Move::Walk;
	cmd.x = static_cast<std::uint8_t>(x);
	cmd.y = static_cast<std::uint8_t>(y);
	cmd.direction = direction;
	return cmd;
}

protocol::Command pick(protocol::Direction direction) {
	protocol::Command cmd;
	cmd.move = protocol::Move::Pick;
	cmd.x = 0;
	cmd.y = 0;
	cmd.direction = direction;
	return cmd;
}

TEST(MockGameClientTest, StartsWithLocalPlayerEntity) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	const game::LocalWorldView& view = client.worldState();
	ASSERT_EQ(view.entities.size(), 1u);
	EXPECT_EQ(view.entities[0].cellX, 5);
	EXPECT_EQ(view.entities[0].cellY, 5);
}

TEST(MockGameClientTest, WalkMovesToProposedCell) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	client.sendCommand(walk(6, 5, protocol::Direction::Right));
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_EQ(player.cellX, 6);
	EXPECT_EQ(player.cellY, 5);
	EXPECT_EQ(player.facing, protocol::Direction::Right);
	EXPECT_TRUE(player.moving);
}

TEST(MockGameClientTest, WalkToAnyCellIsAbsolute) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	client.sendCommand(walk(2, 8, protocol::Direction::Left));
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_EQ(player.cellX, 2);
	EXPECT_EQ(player.cellY, 8);
}

TEST(MockGameClientTest, ClampsTargetOutsideGrid) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	client.sendCommand(walk(99, 99, protocol::Direction::Right));
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_EQ(player.cellX, 9);
	EXPECT_EQ(player.cellY, 9);
}

TEST(MockGameClientTest, WalkToSameCellIsNotMoving) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	client.sendCommand(walk(5, 5, protocol::Direction::Front));
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_FALSE(player.moving);
	EXPECT_EQ(player.facing, protocol::Direction::Front);
}

TEST(MockGameClientTest, PickDoesNotMove) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
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
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_EQ(player.kind, world::CreatureKind::Player);
}

TEST(MockGameClientTest, ProvidesInitialInventoryTools) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	const std::vector<game::ViewInventorySlot>& inv =
		client.worldState().inventory;
	ASSERT_EQ(inv.size(), 4u);
	EXPECT_EQ(inv[0].slotIndex, 0);
	EXPECT_EQ(inv[0].contentId, "pickaxe");
	EXPECT_EQ(inv[0].count, 1);
	EXPECT_EQ(inv[1].slotIndex, 1);
	EXPECT_EQ(inv[1].contentId, "shovel");
	EXPECT_EQ(inv[2].slotIndex, 2);
	EXPECT_EQ(inv[2].contentId, "watering_can");
	EXPECT_EQ(inv[3].slotIndex, 3);
	EXPECT_EQ(inv[3].contentId, "sword");
}

TEST(MockGameClientTest, InventoryStaysWhenPlayerMoves) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	client.sendCommand(walk(6, 5, protocol::Direction::Right));
	EXPECT_EQ(client.worldState().inventory.size(), 4u);
}

TEST(MockGameClientTest, WorldStateReportsGridDimensions) {
	game::MockGameClient client(10, 7, 5, 5, kToolTicks);
	const game::LocalWorldView& view = client.worldState();
	EXPECT_EQ(view.grid.columns, 10);
	EXPECT_EQ(view.grid.rows, 7);
}

TEST(MockGameClientTest, ActiveToolIsNulloptInitially) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_FALSE(player.activeTool.has_value());
}

TEST(MockGameClientTest, ActiveToolIsPickaxeWhenUsingTool) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	client.sendCommand(pick(protocol::Direction::Back));
	const game::ViewEntity& player = client.worldState().entities[0];
	ASSERT_TRUE(player.activeTool.has_value());
	EXPECT_EQ(*player.activeTool, world::ItemKind::Pickaxe);
	EXPECT_EQ(player.action, world::ActionKind::UseTool);
}

TEST(MockGameClientTest, ReturnsToIdleAfterToolUseDuration) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	client.sendCommand(pick(protocol::Direction::Back));
	// Consumir toda la ventana del golpe (una consulta por tick).
	for (int i = 0; i < kToolTicks; ++i) {
		client.worldState();
	}
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_EQ(player.action, world::ActionKind::Idle);
	EXPECT_FALSE(player.activeTool.has_value());
	// El facing se mantiene hacia donde picó.
	EXPECT_EQ(player.facing, protocol::Direction::Back);
}

TEST(MockGameClientTest, ActiveToolClearsAfterWalking) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	client.sendCommand(pick(protocol::Direction::Back));
	client.sendCommand(walk(6, 5, protocol::Direction::Right));
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_FALSE(player.activeTool.has_value());
}

// El mineral de prueba queda en (startX+1, startY): con (10,10,5,5) -> (6,5).
protocol::Command pickAt(int x, int y, protocol::Direction direction) {
	protocol::Command cmd;
	cmd.move = protocol::Move::Pick;
	cmd.x = static_cast<std::uint8_t>(x);
	cmd.y = static_cast<std::uint8_t>(y);
	cmd.direction = direction;
	return cmd;
}

// Busca un mineral por celda en la vista; devuelve su estado, o nullopt
// si en esa celda no hay (o se agotó).
std::optional<world::ResourceState> stateAt(const game::MockGameClient& client,
					    int cellX, int cellY) {
	for (const game::ViewResource& r : client.worldState().resources) {
		if (r.cellX == cellX && r.cellY == cellY) {
			return r.state;
		}
	}
	return std::nullopt;
}

TEST(MockGameClientTest, StartsWithSeveralIntactMinerals) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	const std::vector<game::ViewResource>& res =
		client.worldState().resources;
	// Hay varias piedras y todas arrancan intactas.
	EXPECT_GT(res.size(), 1u);
	for (const game::ViewResource& r : res) {
		EXPECT_EQ(r.state, world::ResourceState::Intact);
	}
	// Una conocida: la plata a la derecha del jugador (6,5).
	EXPECT_EQ(stateAt(client, 6, 5), world::ResourceState::Intact);
}

// Consume la ventana del golpe (una consulta por tick) para que el mock
// confirme el daño al final, como hará el servidor.
void completeHit(game::MockGameClient& client) {
	for (int i = 0; i < kToolTicks; ++i) {
		client.worldState();
	}
}

TEST(MockGameClientTest, DamageAppliesOnlyAfterCompletingTheHit) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	client.sendCommand(pickAt(6, 5, protocol::Direction::Right));
	// Apenas apretado, el mineral sigue intacto (el daño no es inmediato).
	EXPECT_EQ(stateAt(client, 6, 5), world::ResourceState::Intact);
	completeHit(client);
	EXPECT_EQ(stateAt(client, 6, 5), world::ResourceState::Cracked);
}

TEST(MockGameClientTest, PickingMineralAdvancesItsState) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	client.sendCommand(pickAt(6, 5, protocol::Direction::Right));
	completeHit(client);
	EXPECT_EQ(stateAt(client, 6, 5), world::ResourceState::Cracked);
	client.sendCommand(pickAt(6, 5, protocol::Direction::Right));
	completeHit(client);
	EXPECT_EQ(stateAt(client, 6, 5), world::ResourceState::Broken);
}

TEST(MockGameClientTest, MineralDisappearsAfterThirdHit) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	for (int hit = 0; hit < 3; ++hit) {
		client.sendCommand(pickAt(6, 5, protocol::Direction::Right));
		completeHit(client);
	}
	// La piedra golpeada desaparece, pero las otras siguen.
	EXPECT_FALSE(stateAt(client, 6, 5).has_value());
	EXPECT_FALSE(client.worldState().resources.empty());
}

TEST(MockGameClientTest, PickingOneMineralLeavesOthersIntact) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	client.sendCommand(pickAt(6, 5, protocol::Direction::Right));
	completeHit(client);
	// La golpeada avanzó, pero otra piedra (ej. 7,5) sigue intacta.
	EXPECT_EQ(stateAt(client, 6, 5), world::ResourceState::Cracked);
	EXPECT_EQ(stateAt(client, 7, 5), world::ResourceState::Intact);
}

// Agota la piedra en (6,5) con 3 golpes completos.
void exhaustMineral(game::MockGameClient& client) {
	for (int hit = 0; hit < 3; ++hit) {
		client.sendCommand(pickAt(6, 5, protocol::Direction::Right));
		completeHit(client);
	}
}

// Busca un dropped item por celda; devuelve su contentId o nullopt.
std::optional<std::string> droppedAt(const game::MockGameClient& client,
				     int cellX, int cellY) {
	for (const game::ViewDroppedItem& d :
	     client.worldState().droppedItems) {
		if (d.cellX == cellX && d.cellY == cellY) {
			return d.contentId;
		}
	}
	return std::nullopt;
}

protocol::Command take(int x, int y, protocol::Direction direction) {
	protocol::Command cmd;
	cmd.move = protocol::Move::Take;
	cmd.x = static_cast<std::uint8_t>(x);
	cmd.y = static_cast<std::uint8_t>(y);
	cmd.direction = direction;
	return cmd;
}

TEST(MockGameClientTest, ExhaustingMineralDropsItemOnTheFloor) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	// Antes de agotar no hay nada en el piso.
	EXPECT_FALSE(droppedAt(client, 6, 5).has_value());
	exhaustMineral(client);
	// Al agotarse, el item cae en su celda con su contentId genérico.
	EXPECT_EQ(droppedAt(client, 6, 5), "silver");
}

TEST(MockGameClientTest, ExhaustingMineralDoesNotEmitObtainedAutomatically) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	exhaustMineral(client);
	// Ya no hay obtención automática: hay que recoger con Take.
	for (int i = 0; i < 60; ++i) {
		EXPECT_TRUE(client.worldState().obtained.empty());
	}
}

TEST(MockGameClientTest, TakeOnDroppedCellEmitsObtained) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	exhaustMineral(client);
	// El jugador se para sobre la celda del item y lo recoge.
	client.sendCommand(take(6, 5, protocol::Direction::Right));
	const game::LocalWorldView& view = client.worldState();
	ASSERT_EQ(view.obtained.size(), 1u);
	EXPECT_EQ(view.obtained.front().kind, world::ResourceKind::Silver);
	// El item desaparece del piso al recogerse.
	EXPECT_FALSE(droppedAt(client, 6, 5).has_value());
}

TEST(MockGameClientTest, TakeOnEmptyCellEmitsNothing) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	// No hay item dropeado todavía: recoger no emite nada.
	client.sendCommand(take(5, 5, protocol::Direction::Front));
	EXPECT_TRUE(client.worldState().obtained.empty());
}

TEST(MockGameClientTest, ObtainedFromTakeLastsOnlyOneTick) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	exhaustMineral(client);
	client.sendCommand(take(6, 5, protocol::Direction::Right));
	// Primer estado: trae el evento.
	ASSERT_FALSE(client.worldState().obtained.empty());
	// El tick siguiente ya no lo tiene (es puntual, no persistente).
	EXPECT_TRUE(client.worldState().obtained.empty());
}

// Busca un slot del inventario por contentId; devuelve su count o nullopt.
std::optional<int> inventoryCount(const game::MockGameClient& client,
				  const std::string& contentId) {
	for (const game::ViewInventorySlot& s :
	     client.worldState().inventory) {
		if (s.contentId == contentId) {
			return s.count;
		}
	}
	return std::nullopt;
}

// Agota la piedra en (cx,cy) con 3 golpes completos (apuntando a esa celda).
void exhaustMineralAt(game::MockGameClient& client, int cx, int cy) {
	for (int hit = 0; hit < 3; ++hit) {
		client.sendCommand(pickAt(cx, cy, protocol::Direction::Right));
		completeHit(client);
	}
}

TEST(MockGameClientTest, TakingMineralAddsItToInventoryWithCountOne) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	EXPECT_FALSE(inventoryCount(client, "silver").has_value());
	exhaustMineral(client);  // silver en (6,5)
	client.sendCommand(take(6, 5, protocol::Direction::Right));
	EXPECT_EQ(inventoryCount(client, "silver"), 1);
	// Las herramientas siguen presentes.
	EXPECT_EQ(inventoryCount(client, "pickaxe"), 1);
	EXPECT_EQ(inventoryCount(client, "shovel"), 1);
}

TEST(MockGameClientTest, TakingSameMineralTwiceAccumulatesCount) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	// Dos piedras de plata: (6,5) y (startX, startY+2) = (5,7).
	exhaustMineral(client);  // silver (6,5)
	client.sendCommand(take(6, 5, protocol::Direction::Right));
	EXPECT_EQ(inventoryCount(client, "silver"), 1);
	exhaustMineralAt(client, 5, 7);  // silver (5,7)
	client.sendCommand(take(5, 7, protocol::Direction::Right));
	EXPECT_EQ(inventoryCount(client, "silver"), 2);
}

TEST(MockGameClientTest, TakingDifferentMineralsUsesDifferentSlots) {
	game::MockGameClient client(10, 10, 5, 5, kToolTicks);
	exhaustMineral(client);  // silver (6,5)
	client.sendCommand(take(6, 5, protocol::Direction::Right));
	exhaustMineralAt(client, 7, 5);  // coal (7,5)
	client.sendCommand(take(7, 5, protocol::Direction::Right));
	EXPECT_EQ(inventoryCount(client, "silver"), 1);
	EXPECT_EQ(inventoryCount(client, "coal"), 1);
	// Slots distintos (índices libres tras las 4 herramientas).
	std::optional<int> silverSlot;
	std::optional<int> coalSlot;
	for (const game::ViewInventorySlot& s :
	     client.worldState().inventory) {
		if (s.contentId == "silver") {
			silverSlot = s.slotIndex;
		}
		if (s.contentId == "coal") {
			coalSlot = s.slotIndex;
		}
	}
	ASSERT_TRUE(silverSlot.has_value());
	ASSERT_TRUE(coalSlot.has_value());
	EXPECT_NE(*silverSlot, *coalSlot);
}

}
