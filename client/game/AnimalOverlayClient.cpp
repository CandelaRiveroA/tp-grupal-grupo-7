#include "game/AnimalOverlayClient.h"

#include <array>
#include <span>
#include <string>
#include <utility>

#include "game/GameConstants.h"
#include "game/MockAnimalSimulation.h"
#include "world/ItemKind.h"

namespace game {

namespace {

// Datos del "server falso": seed y celdas de spawn de los animales
// (réplica del modelo World::set_entity_at del server). Es dato de la
// simulación, no presentación: cuando el server tenga animales, esta
// capa desaparece y estos datos pasan al server.
constexpr unsigned int kAnimalSeed = 20261027;

// Celdas de spawn en la sala inicial (grilla 30x20, jugador al centro).
constexpr std::array<world::GridCell, 3> kChickenSpawns = {{
	{3, 3},
	{26, 4},
	{12, 2},
}};

// Una fila por especie de la simulación: kind + sus celdas (adentro de
// la grilla, adyacentes a las de la gallina). Sumar una especie = una
// fila acá + la apariencia en characters.toml.
constexpr std::array<world::GridCell, 3> kCowSpawns = {{
	{4, 4},
	{25, 5},
	{13, 3},
}};

struct AnimalBinding {
	world::CreatureKind kind;
	std::span<const world::GridCell> cells;
};

constexpr std::array<AnimalBinding, 2> kAnimalBindings = {{
	{world::CreatureKind::Chicken, kChickenSpawns},
	{world::CreatureKind::Cow, kCowSpawns},
}};

// Tope de ítems en el suelo: la vista no se llena de huevos (sin
// recogida no hay forma de que se vayan). Regla silenciosa de juego del
// "server falso", como el límite de slots del inventario; el server
// real definirá su propio tope/despawn.
constexpr int kMaxGroundItems = 3;

}

AnimalOverlayClient::AnimalOverlayClient(
	std::unique_ptr<GameClient> inner,
	const world::CharacterCatalog& characters, world::GridSpan grid)
	: inner_(std::move(inner))
	, simulation_(std::make_unique<MockAnimalSimulation>(
		  grid.columns, grid.rows, kAnimalSeed))
	, grid_(grid) {
	spawnAnimals(characters);
	mergeView();
}

AnimalOverlayClient::~AnimalOverlayClient() = default;

void AnimalOverlayClient::sendCommand(const protocol::ClientMessage& message) {
	inner_->sendCommand(message);
}

void AnimalOverlayClient::update(int iteration) {
	inner_->update(iteration);

	int playerX = grid_.columns / 2;
	int playerY = grid_.rows / 2;
	playerCell(playerX, playerY);
	simulation_->update(iteration, playerX, playerY);
	collectProducedItems();

	// Los sonidos del tick: eventos PER-TICK (drena, no acumulan).
	soundEvents_ = simulation_->takeSoundEvents();

	mergeView();
}

const LocalWorldView& AnimalOverlayClient::worldState() const {
	return merged_;
}

void AnimalOverlayClient::playerCell(int& cellX, int& cellY) const {
	const LocalWorldView& state = inner_->worldState();
	if (state.entities.empty()) {
		return;
	}
	cellX = state.entities[kLocalPlayerIndex].cellX;
	cellY = state.entities[kLocalPlayerIndex].cellY;
}

void AnimalOverlayClient::spawnAnimals(
	const world::CharacterCatalog& characters) {
	int playerX = grid_.columns / 2;
	int playerY = grid_.rows / 2;
	playerCell(playerX, playerY);

	for (const AnimalBinding& binding : kAnimalBindings) {
		// Sin apariencia declarada, el kind no entra: si characters
		// no lo describe, el render no lo sabría dibujar.
		const world::CreatureData* creature =
			characters.find(binding.kind);
		if (creature == nullptr) {
			continue;
		}
		for (const world::GridCell& cell : binding.cells) {
			simulation_->spawn(binding.kind, creature->cadence, cell,
					   playerX, playerY);
		}
	}
}

void AnimalOverlayClient::collectProducedItems() {
	for (ProducedItem& laid : simulation_->takeProducedItems()) {
		if (static_cast<int>(groundItems_.size()) >= kMaxGroundItems) {
			break;
		}
		bool occupied = false;
		for (const ViewDroppedItem& item : groundItems_) {
			occupied = occupied ||
				   (item.cellX == laid.cellX &&
				    item.cellY == laid.cellY);
		}
		if (occupied) {
			// La celda ya tiene un ítem: la producción se
			// descarta (regla silenciosa, no un error).
			continue;
		}
		groundItems_.push_back(
			ViewDroppedItem{contentIdFor(laid.item), laid.cellX,
					laid.cellY});
	}
}

std::string AnimalOverlayClient::contentIdFor(world::ItemKind kind) const {
	switch (kind) {
		case world::ItemKind::Shovel:
			return "shovel";
		case world::ItemKind::Pickaxe:
			return "pickaxe";
		case world::ItemKind::WateringCan:
			return "watering_can";
		case world::ItemKind::Sword:
			return "sword";
		case world::ItemKind::Egg:
			return "egg";
		case world::ItemKind::Milk:
			return "milk";
	}
	return "";
}

void AnimalOverlayClient::mergeView() {
	merged_ = inner_->worldState();

	if (simulation_ != nullptr) {
		// Animales después de las entidades del inner, en orden de
		// spawn (= orden de id, estable entre frames).
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
			merged_.entities.push_back(entity);
		}

		merged_.soundEvents.insert(merged_.soundEvents.end(),
					  soundEvents_.begin(),
					  soundEvents_.end());
	}

	// Productos de la capa: al final de los ítems del suelo del inner.
	for (const ViewDroppedItem& item : groundItems_) {
		merged_.droppedItems.push_back(item);
	}
}

}  // namespace game
