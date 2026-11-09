#include "game/EnemyBot.h"

#include <algorithm>
#include <array>
#include <span>

#include "game/GameConstants.h"
// DirectionDelta (dirección -> delta en celdas): el mismo DTO de datos
// del movimiento del mundo simulado (reutilización, §9).
#include "game/MockAnimalSimulation.h"

namespace game {

namespace {

// Celdas de spawn de los enemigos en la sala inicial (grilla 30x20,
// jugador al centro). Datos del "server falso": cuando el server tenga
// enemigos, pasan al server y esta capa desaparece.
constexpr std::array<world::GridCell, 2> kOrcSpawns = {{
	{15, 9},
	{16, 9},
}};

constexpr std::array<world::GridCell, 2> kSkeletonSpawns = {{
	{6, 14},
	{23, 15},
}};

constexpr std::array<world::GridCell, 2> kVampireSpawns = {{
	{10, 16},
	{19, 7},
}};

// Una fila por especie del bot: kind + sus celdas. Sumar una especie =
// una fila acá + su apariencia en enemies.toml (convención §4).
struct EnemyBinding {
	world::CreatureKind kind;
	std::span<const world::GridCell> cells;
};

constexpr std::array<EnemyBinding, 3> kEnemyBindings = {{
	{world::CreatureKind::Orc, kOrcSpawns},
	{world::CreatureKind::Skeleton, kSkeletonSpawns},
	{world::CreatureKind::Vampire, kVampireSpawns},
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

EnemyBot::EnemyBot(const world::CharacterCatalog& characters,
		   const world::GridSpan grid)
	: characters_(&characters)
	, grid_(grid)
	, activityForAction_{
		  {world::ActionKind::Walk, EnemyActivity::Walking},
		  {world::ActionKind::Attack, EnemyActivity::Attacking},
		  {world::ActionKind::Damage, EnemyActivity::Damaged},
		  {world::ActionKind::Death, EnemyActivity::Dead},
	  }
	// Prioridad de cada actividad: solo se acepta una acción de
	// prioridad MAYOR que la actividad actual (sin empates: una acción
	// a la vez). El daño interrumpe un ataque; la caminata no interrumpe
	// nada; la muerte interrumpe todo y es terminal.
	, activityPriority_{
		  {EnemyActivity::Idle, 0},
		  {EnemyActivity::Walking, 1},
		  {EnemyActivity::Attacking, 2},
		  {EnemyActivity::Damaged, 3},
		  {EnemyActivity::Dead, 4},
	  } {}

void EnemyBot::spawnEnemies(const world::CharacterCatalog& characters,
			    const int playerCellX,
			    const int playerCellY,
			    const std::vector<world::CreatureKind>& only) {
	for (const EnemyBinding& binding : kEnemyBindings) {
		// Filtro opcional: con `only` no vacío, spawnea SOLO esas
		// especies (el resto las trae otra fuente, p. ej. el
		// server); vacío = todas (el mock standalone y sus tests).
		if (!only.empty() &&
		    std::find(only.begin(), only.end(), binding.kind) ==
			    only.end()) {
			continue;
		}
		// Sin apariencia declarada, el kind no entra: si characters
		// no lo describe, el render no lo sabría dibujar.
		if (characters.find(binding.kind) == nullptr) {
			continue;
		}
		for (const world::GridCell& cell : binding.cells) {
			EnemyRecord enemy;
			enemy.id = kEnemyBotFirstId +
				   static_cast<int>(enemies_.size());
			enemy.kind = binding.kind;
			enemy.cellX = cell.column;
			enemy.cellY = cell.row;
			if (!canEnter(enemy.cellX, enemy.cellY, playerCellX,
				      playerCellY, enemy)) {
				continue;
			}
			enemies_.push_back(enemy);
		}
	}
}

void EnemyBot::triggerAll(const world::ActionKind action,
			  const int iteration,
			  const int playerCellX,
			  const int playerCellY) {
	const auto activity = activityForAction_.find(action);
	if (activity == activityForAction_.end()) {
		return;
	}
	for (EnemyRecord& enemy : enemies_) {
		// Una acción a la vez: solo gana la de prioridad
		// ESTRICTAMENTE mayor que la actividad actual.
		if (activityPriority_.at(activity->second) <=
		    activityPriority_.at(enemy.activity)) {
			continue;
		}
		// El kind tiene que tener la animación (el orco no tiene
		// daño ni muerte: se saltea). Walk siempre existe.
		if (action != world::ActionKind::Walk &&
		    characters_->dataFor(enemy.kind).actionCycleTicks(action) <=
			    0) {
			continue;
		}
		if (activity->second == EnemyActivity::Walking) {
			tryStep(enemy, iteration, playerCellX, playerCellY);
			continue;
		}
		// Ataque / daño / muerte: la entidad queda quieta y abre la
		// ventana de la acción (el resolver anima desde el frame 0).
		enemy.moving = false;
		enemy.action = action;
		enemy.activity = activity->second;
		enemy.endIteration =
			iteration + characters_->dataFor(enemy.kind)
					    .actionWindowTicks(action);
	}
}

bool EnemyBot::tryStep(EnemyRecord& enemy, const int iteration,
		       const int playerCellX, const int playerCellY) {
	// La dirección actual primero y después las otras en orden de
	// tabla; el primer destino pisable arranca el paso.
	std::size_t start = 0;
	for (std::size_t i = 0; i < kDirectionDeltas.size(); ++i) {
		if (kDirectionDeltas[i].direction == enemy.facing) {
			start = i;
			break;
		}
	}
	for (std::size_t n = 0; n < kDirectionDeltas.size(); ++n) {
		const DirectionDelta& delta =
			kDirectionDeltas[(start + n) % kDirectionDeltas.size()];
		const int cellX = enemy.cellX + delta.deltaX;
		const int cellY = enemy.cellY + delta.deltaY;
		if (!canEnter(cellX, cellY, playerCellX, playerCellY, enemy)) {
			continue;
		}
		enemy.cellX = cellX;
		enemy.cellY = cellY;
		enemy.facing = delta.direction;
		enemy.moving = true;
		enemy.action = world::ActionKind::Walk;
		enemy.activity = EnemyActivity::Walking;
		enemy.endIteration =
			iteration +
			characters_->dataFor(enemy.kind).cadence.stepTicks;
		return true;
	}
	// Sin celda libre no avanza: se queda como está (Idle).
	return false;
}

bool EnemyBot::canEnter(const int cellX, const int cellY,
			const int playerCellX, const int playerCellY,
			const EnemyRecord& self) const {
	if (cellX < 0 || cellX >= grid_.columns || cellY < 0 ||
	    cellY >= grid_.rows) {
		return false;
	}
	if (cellX == playerCellX && cellY == playerCellY) {
		return false;
	}
	for (const EnemyRecord& other : enemies_) {
		if (other.id != self.id && other.cellX == cellX &&
		    other.cellY == cellY) {
			return false;
		}
	}
	return true;
}

void EnemyBot::update(const int iteration) {
	for (EnemyRecord& enemy : enemies_) {
		// Idle no tiene ventana; Dead es terminal (se queda en el
		// último frame de la muerte).
		if (enemy.activity == EnemyActivity::Idle ||
		    enemy.activity == EnemyActivity::Dead) {
			continue;
		}
		if (iteration < enemy.endIteration) {
			continue;
		}
		enemy.moving = false;
		enemy.action = world::ActionKind::Idle;
		enemy.activity = EnemyActivity::Idle;
	}
}

void EnemyBot::appendEntities(std::vector<ViewEntity>& out) const {
	for (const EnemyRecord& enemy : enemies_) {
		ViewEntity entity;
		entity.id = enemy.id;
		entity.kind = enemy.kind;
		entity.cellX = enemy.cellX;
		entity.cellY = enemy.cellY;
		entity.facing = enemy.facing;
		entity.moving = enemy.moving;
		entity.action = enemy.action;
		// El "server falso" no los marca interactivos (no se
		// seleccionan); mañana lo mandará el server real.
		entity.interactive = false;
		out.push_back(entity);
	}
}

}  // namespace game
