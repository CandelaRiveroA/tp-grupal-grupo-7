#include "game/MockGameClient.h"

#include <algorithm>
#include <array>

#include "game/MockAnimalSimulation.h"
#include "world/CreatureCatalog.h"
#include "world/Grid.h"

namespace game {

namespace {

// Datos del "server falso": seed y celdas de spawn de los animales
// (réplica del modelo World::set_entity_at del server). Es dato del
// server simulado, no presentación: al conectar la red, quien llena la
// vista pasa a ser la implementación de red y esto queda reemplazado.
constexpr unsigned int kAnimalSeed = 20261027;

// Celdas de spawn en la sala inicial (grilla 30x20, jugador al centro).
constexpr std::array<world::GridCell, 3> kAnimalSpawns = {{
	{3, 3},
	{26, 4},
	{12, 2},
}};

// Tope de ítems en el suelo: la vista no se llena de huevos (sin
// recogida no hay forma de que se vayan). Regla silenciosa de juego del
// "server falso", como el límite de slots del inventario; el server
// real definirá su propio tope/despawn.
constexpr int kMaxGroundItems = 3;

}

MockGameClient::MockGameClient(int gridColumns, int gridRows, int startX,
			       int startY)
	: gridColumns_(gridColumns)
	, gridRows_(gridRows)
	, playerCellX_(startX)
	, playerCellY_(startY)
	, playerFacing_(protocol::Direction::Front)
	, playerMoving_(false)
	, simulation_(nullptr)
	, nextGroundItemId_(1) {
	rebuildInventory();
	rebuildView();
}

MockGameClient::MockGameClient(int gridColumns, int gridRows, int startX,
			       int startY,
			       const world::CharacterCatalog& characters)
	: gridColumns_(gridColumns)
	, gridRows_(gridRows)
	, playerCellX_(startX)
	, playerCellY_(startY)
	, playerFacing_(protocol::Direction::Front)
	, playerMoving_(false)
	, simulation_(std::make_unique<MockAnimalSimulation>(gridColumns,
							 gridRows,
							 kAnimalSeed))
	, nextGroundItemId_(1) {
	spawnAnimals(characters);
	rebuildInventory();
	rebuildView();
}

MockGameClient::~MockGameClient() = default;

void MockGameClient::sendCommand(const protocol::Command& command) {
	switch (command.move) {
	case protocol::Move::Walk:
		walkTo(command.x, command.y, command.direction);
		break;
	case protocol::Move::Pick:
		playerMoving_ = false;
		playerFacing_ = command.direction;
		break;
	}
	rebuildView();
}

void MockGameClient::update(int iteration) {
	if (simulation_ == nullptr) {
		return;
	}
	simulation_->update(iteration, playerCellX_, playerCellY_);
	collectLaidEggs();
	rebuildView();
	// Los cacareos del tick: eventos PER-TICK (drena, no acumula).
	// Quedan en la vista hasta el próximo update: sendCommand no
	// los borra, así el loop los lee aunque el jugador camine.
	view_.soundEvents = simulation_->takeClucks();
}

const LocalWorldView& MockGameClient::worldState() const { return view_; }

void MockGameClient::walkTo(int targetX, int targetY,
			    protocol::Direction direction) {
	const int clampedX = std::clamp(targetX, 0, gridColumns_ - 1);
	const int clampedY = std::clamp(targetY, 0, gridRows_ - 1);

	playerMoving_ =
		(clampedX != playerCellX_ || clampedY != playerCellY_);
	playerCellX_ = clampedX;
	playerCellY_ = clampedY;
	playerFacing_ = direction;
}

void MockGameClient::rebuildView() {
	view_.entities.clear();
	view_.grid = world::GridSpan{gridColumns_, gridRows_};
	view_.groundItems = groundItems_;

	// El jugador es SIEMPRE la entidad 0 con id estable 0.
	ViewEntity player;
	player.kind = world::CreatureKind::Player;
	player.cellX = playerCellX_;
	player.cellY = playerCellY_;
	player.facing = playerFacing_;
	player.moving = playerMoving_;
	player.action = playerMoving_ ? world::ActionKind::Walk
				      : world::ActionKind::Idle;
	player.id = 0;
	view_.entities.push_back(player);

	if (simulation_ == nullptr) {
		return;
	}
	// Animales después del jugador, en orden de spawn (= orden de id,
	// estable entre frames: el requisito de "id estable" del estado).
	for (const AnimalRecord& animal : simulation_->animals()) {
		ViewEntity entity;
		entity.id = animal.id;
		entity.kind = animal.kind;
		entity.cellX = animal.cellX;
		entity.cellY = animal.cellY;
		entity.facing = animal.facing;
		entity.moving = animal.moving;
		entity.action = animal.moving ? world::ActionKind::Walk
					      : world::ActionKind::Idle;
		view_.entities.push_back(entity);
	}
}

void MockGameClient::collectLaidEggs() {
	for (LaidEgg& laid : simulation_->takeLaidEggs()) {
		if (static_cast<int>(groundItems_.size()) >=
		    kMaxGroundItems) {
			break;
		}
		bool occupied = false;
		for (const ViewGroundItem& item : groundItems_) {
			occupied = occupied ||
				   (item.cellX == laid.cellX &&
				    item.cellY == laid.cellY);
		}
		if (occupied) {
			// La celda ya tiene un huevo: la postura se
			// descarta (regla silenciosa, no un error).
			continue;
		}

		ViewGroundItem egg;
		egg.id = nextGroundItemId_;
		egg.cellX = laid.cellX;
		egg.cellY = laid.cellY;
		egg.item = laid.item;
		groundItems_.push_back(egg);
		nextGroundItemId_ += 1;
	}
}

void MockGameClient::rebuildInventory() {
	view_.inventory.clear();
	view_.inventory.push_back({0, world::ItemKind::Shovel});
	view_.inventory.push_back({1, world::ItemKind::Pickaxe});
	view_.inventory.push_back({2, world::ItemKind::Sword});
	view_.inventory.push_back({3, world::ItemKind::WateringCan});
}

void MockGameClient::spawnAnimals(
	const world::CharacterCatalog& characters) {
	if (!characters.has(world::CreatureKind::Chicken)) {
		return;
	}
	const world::MoveCadence& cadence =
		characters.dataFor(world::CreatureKind::Chicken).cadence;
	for (const world::GridCell& cell : kAnimalSpawns) {
		simulation_->spawn(world::CreatureKind::Chicken, cadence,
				   cell, playerCellX_, playerCellY_);
	}
}

}