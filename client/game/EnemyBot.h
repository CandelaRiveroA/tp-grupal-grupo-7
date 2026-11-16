#pragma once

#include <map>
#include <optional>
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
	// Revivir: la muerte al revés. Única que le gana a Dead (la pone
	// en marcha la tecla de prueba; mañana la fuente).
	Reviving,
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

// Estado de UN proyectil del "server falso": DATOS puros. No tiene IA
// propia: existe, avanza cuando la fuente lo dice y desaparece. Se
// dibuja por el mismo camino que una entidad (kind + celda + facing).
struct ProjectileRecord {
	int id;
	world::CreatureKind kind;
	int cellX;
	int cellY;
	world::Direction facing = world::Direction::Front;
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

	// Ataque con un proyectil CONCRETO (hoy el fuego verde de prueba):
	// solo atacan los enemigos que declaran poder lanzarlo, y disparan
	// ese proyectil en vez del default. Misma tabla de prioridades.
	void triggerAttackWith(world::CreatureKind projectile, int iteration,
			       int playerCellX, int playerCellY);

	// Cierra las ventanas vencidas (vuelven a Idle).
	void update(int iteration);

	// ANDAMIAJE TEMPORAL (prueba): mueve cada proyectil una celda en su
	// dirección, o los borra. Simula lo que mañana mandará el server
	// (la celda de cada proyectil). Solo cliente.
	void advanceProjectiles();
	void despawnProjectiles();

	// Las entidades de los enemigos y sus proyectiles para la vista
	// (después de los animales de granja).
	void appendEntities(std::vector<ViewEntity>& out) const;

private:
	// Aplica UNA acción a UN enemigo (si su prioridad gana). Con
	// `projectile` fuerza ese proyectil en el ataque; sin él usa el
	// default de la tabla.
	void applyAction(EnemyRecord& enemy, EnemyActivity activity,
			 world::ActionKind action, int iteration,
			 int playerCellX, int playerCellY,
			 std::optional<world::CreatureKind> projectile);
	// El paso intenta la dirección actual primero y después las otras
	// en orden de tabla. Devuelve si arrancó el paso.
	bool tryStep(EnemyRecord& enemy, int iteration, int playerCellX,
		     int playerCellY);

	bool canEnter(int cellX, int cellY, int playerCellX, int playerCellY,
		      const EnemyRecord& self) const;

	// Crea un proyectil en la celda si cae dentro de la grilla.
	void launchProjectile(world::CreatureKind kind, int cellX, int cellY,
			      world::Direction facing);

	// Delta en celdas de una dirección (la MISMA tabla del paso).
	world::GridCell deltaFor(world::Direction facing) const;

	const world::CharacterCatalog* characters_;
	world::GridSpan grid_;
	// Acción -> actividad que enciende; y prioridad de cada actividad
	// (la de la actividad actual se compara con la destino). Sumar una
	// acción nueva = una fila en cada tabla (convención §4/§9).
	std::map<world::ActionKind, EnemyActivity> activityForAction_;
	std::map<EnemyActivity, int> activityPriority_;
	// Enemigo -> proyectiles que puede lanzar al atacar (el PRIMERO es
	// el default). Sumar un tipo de fuego = una entrada en la lista +
	// su apariencia en enemies.toml.
	std::map<world::CreatureKind, std::vector<world::CreatureKind>>
		attackProjectiles_;

	std::vector<EnemyRecord> enemies_;
	// Proyectiles en vuelo (hoy el fuego del Skull): estado del "server
	// falso", sin IA propia.
	std::vector<ProjectileRecord> projectiles_;
	int nextProjectileId_;
};

}  // namespace game
