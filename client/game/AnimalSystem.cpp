#include "game/AnimalSystem.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <span>
#include <utility>

#include "game/GameConstants.h"
#include "game/MockAnimalSimulation.h"

namespace game {

namespace {

// Celdas de spawn en la sala inicial (grilla 30x20, jugador al centro).
// Datos del "server falso": cuando el server tenga animales, pasan al
// server y esta capa desaparece.
constexpr std::array<world::GridCell, 3> kChickenSpawns = {{
	{3, 3},
	{26, 4},
	{12, 2},
}};

constexpr std::array<world::GridCell, 3> kCowSpawns = {{
	{4, 4},
	{25, 5},
	{13, 3},
}};

// Una fila por especie de la simulación: kind + sus celdas. Sumar una
// especie = una fila acá + la apariencia en characters.toml. Los
// enemigos NO van acá: tienen su propio módulo (EnemyBot).
struct AnimalBinding {
	world::CreatureKind kind;
	std::span<const world::GridCell> cells;
};

constexpr std::array<AnimalBinding, 2> kAnimalBindings = {{
	{world::CreatureKind::Chicken, kChickenSpawns},
	{world::CreatureKind::Cow, kCowSpawns},
}};

// Productos cuya "cosecha" deja el animal LISTO en vez de tirarlos al
// piso (la leche se ordeña). Tabla, no switch (convención §4): sumar
// un producto es una entrada.
constexpr std::array<world::ItemKind, 1> kReadyProducts = {{
	world::ItemKind::Milk,
}};

}

AnimalSystem::AnimalSystem(const world::GridSpan grid, FloorItems& floor)
	: simulation_(std::make_unique<MockAnimalSimulation>(
		  grid.columns, grid.rows, kAnimalSeed))
	, floor_(&floor) {}

AnimalSystem::~AnimalSystem() = default;

void AnimalSystem::spawnAnimals(const world::CharacterCatalog& characters,
				const int playerCellX,
				const int playerCellY,
				const std::vector<world::CreatureKind>& only) {
	for (const AnimalBinding& binding : kAnimalBindings) {
		// Filtro opcional: con `only` no vacío, spawnea SOLO esas
		// especies (el resto las trae otra fuente, p. ej. el server);
		// vacío = todas (el mock standalone y sus tests).
		if (!only.empty() &&
		    std::find(only.begin(), only.end(), binding.kind) ==
			    only.end()) {
			continue;
		}
		// Sin apariencia declarada, el kind no entra: si characters
		// no lo describe, el render no lo sabría dibujar.
		const world::CreatureData* creature =
			characters.find(binding.kind);
		if (creature == nullptr) {
			continue;
		}
		for (const world::GridCell& cell : binding.cells) {
			simulation_->spawn(binding.kind, creature->cadence,
					   cell, playerCellX, playerCellY);
		}
	}
}

void AnimalSystem::update(const int iteration, const int playerCellX,
			  const int playerCellY) {
	simulation_->update(iteration, playerCellX, playerCellY);
	for (const ProducedItem& laid : simulation_->takeProducedItems()) {
		// Cada producto según su política: la leche deja la vaca
		// lista para ordeñar; el resto cae al piso del mundo.
		bool ready = false;
		for (const world::ItemKind kind : kReadyProducts) {
			ready = ready || laid.item == kind;
		}
		if (ready) {
			readyAnimals_.insert(laid.animalId);
			continue;
		}
		floor_->tryDrop(laid.item, laid.cellX, laid.cellY);
	}
	soundEvents_ = simulation_->takeSoundEvents();
}

void AnimalSystem::appendEntities(std::vector<ViewEntity>& out) const {
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
		// El "server falso" marca lo que es interactivo (hoy los
		// animales); mañana lo mandará el server real en el estado.
		entity.interactive = true;
		if (readyAnimals_.count(animal.id) > 0) {
			entity.indication = world::Indication::ReadyToMilk;
		}
		// La vaca que se está ordeñando queda quieta.
		if (animal.id == frozenCowId_) {
			entity.cellX = frozenCowCellX_;
			entity.cellY = frozenCowCellY_;
			entity.moving = false;
			entity.action = world::ActionKind::Idle;
		}
		out.push_back(entity);
	}
}

std::vector<ViewSoundEvent> AnimalSystem::takeSoundEvents() {
	std::vector<ViewSoundEvent> events = std::move(soundEvents_);
	soundEvents_.clear();
	return events;
}

std::optional<InteractionTarget> AnimalSystem::findTargetNear(
	const int cellX, const int cellY) const {
	for (const AnimalRecord& animal : simulation_->animals()) {
		if (readyAnimals_.count(animal.id) == 0) {
			continue;
		}
		// Cerca = las celdas se tocan (adyacentes o la misma). La
		// regla la decide la fuente de estado.
		if (std::abs(animal.cellX - cellX) > 1 ||
		    std::abs(animal.cellY - cellY) > 1) {
			continue;
		}
		InteractionTarget target;
		target.action = world::ActionKind::Milk;
		target.cycles = kDefaultMilkCycles;
		const std::string* contentId =
			contentIds_.find(world::ItemKind::Milk);
		target.reward = contentId != nullptr ? *contentId : "";
		target.targetId = animal.id;
		target.cellX = animal.cellX;
		target.cellY = animal.cellY;
		return target;
	}
	return std::nullopt;
}

void AnimalSystem::onInteractionStarted(const InteractionTarget& target) {
	// La vaca se congela mientras se la ordeña: misma celda todo el
	// rato. El ordeño no la mueve.
	for (const AnimalRecord& animal : simulation_->animals()) {
		if (animal.id == target.targetId) {
			frozenCowId_ = animal.id;
			frozenCowCellX_ = animal.cellX;
			frozenCowCellY_ = animal.cellY;
			return;
		}
	}
}

void AnimalSystem::onInteractionFinished(const InteractionTarget& target) {
	// La vaca deja de estar lista (su diálogo se cierra) y vuelve a
	// moverse.
	readyAnimals_.erase(target.targetId);
	frozenCowId_ = -1;
}

}  // namespace game
