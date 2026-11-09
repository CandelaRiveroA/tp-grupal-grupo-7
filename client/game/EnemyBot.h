#pragma once

#include <map>
#include <vector>

#include "game/LocalWorldView.h"
#include "world/CharacterCatalog.h"
#include "world/Grid.h"

namespace game {

// Actividad de un enemigo en el tick: la decide el bot (una acción a la
// vez). Datos, sin lógica (convención §1).
enum class EnemyActivity {
	Idle,
	Walking,
	Attacking,
	Damaged,
	Dead,
};

// Estado de UN enemigo: DATOS puros (convención §1); los vuelca a la
// vista igual que el server real lo hará mañana.
struct EnemyRecord {
	int id;
	world::CreatureKind kind;
	int cellX;
	int cellY;
	world::Direction facing = world::Direction::Front;
	bool moving = false;
	world::ActionKind action = world::ActionKind::Idle;
	EnemyActivity activity = EnemyActivity::Idle;
	// Tick en que termina la actividad en curso (la ventana de la
	// acción, que la fija el catálogo: frames x frameTicks).
	int endIteration = 0;
};

// ANDAMIAJE TEMPORAL (borrable de una pieza): el stand-in del server
// para los ENEMIGOS. Produce el mismo estado que el server real mandará
// (game::ViewEntity con id estable, celda, facing, moving y UNA acción)
// y el cliente solo lo anima (contrato §6/§12). Una acción a la vez:
// solo se acepta una acción de prioridad MAYOR que la actividad actual
// (Dead > Damaged > Attacking > Walking > Idle), así un daño interrumpe
// un ataque pero una caminata no. Cuando el server mande enemigos y sus
// acciones, se borra este módulo con sus teclas de prueba.
class EnemyBot {
public:
	EnemyBot(const world::CharacterCatalog& characters, world::GridSpan grid);

	// Spawnea los enemigos del mundo (celdas en el .cpp). El jugador
	// está en su celda para no spawnear encima. `only`: si no está
	// vacío, spawnea SOLO esas especies; vacío = todas.
	void spawnEnemies(const world::CharacterCatalog& characters,
			  int playerCellX, int playerCellY,
			  const std::vector<world::CreatureKind>& only = {});

	// Dispara una acción de prueba sobre todos los enemigos (one-shot):
	// cada uno la acepta solo si su prioridad gana a su actividad
	// actual (tabla en el .cpp). Caminar mueve un paso; el resto abre
	// la ventana de la acción y al terminar vuelve a Idle (Death no:
	// es terminal y se queda en el último frame).
	void triggerAll(world::ActionKind action, int iteration,
			int playerCellX, int playerCellY);

	// Cierra las ventanas vencidas (vuelven a Idle).
	void update(int iteration);

	// Las entidades de los enemigos para la vista (después de los
	// animales de granja).
	void appendEntities(std::vector<ViewEntity>& out) const;

private:
	// El paso intenta la dirección actual primero y después las otras
	// en orden de tabla. Devuelve si arrancó el paso.
	bool tryStep(EnemyRecord& enemy, int iteration, int playerCellX,
		     int playerCellY);

	bool canEnter(int cellX, int cellY, int playerCellX, int playerCellY,
		      const EnemyRecord& self) const;

	const world::CharacterCatalog* characters_;
	world::GridSpan grid_;
	// Acción -> actividad que enciende; y prioridad de cada actividad
	// (la de la actividad actual se compara con la destino). Sumar una
	// acción nueva = una fila en cada tabla (convención §4/§9).
	std::map<world::ActionKind, EnemyActivity> activityForAction_;
	std::map<EnemyActivity, int> activityPriority_;

	std::vector<EnemyRecord> enemies_;
};

}  // namespace game
