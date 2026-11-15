#pragma once

#include "world/CharacterAppearance.h"
#include "world/CreatureCatalog.h"

namespace game {

// ANDAMIAJE TEMPORAL (solo cliente, borrable de una pieza). Disparador
// de prueba para encender acciones de los enemigos con el teclado,
// mientras el server todavía no manda esas acciones. El seam real es
// game::ViewEntity::action (contrato §6/§10): el render ya anima
// cualquier acción que llegue en la vista. Esto SOLO escribe en esa
// misma fuente (hoy el "server falso"), para probar la animación.
//
// Interfaz chica y enfocada (§9): PlaySession depende de esto, no del
// tipo concreto del mundo simulado. El día que el server mande las
// acciones, se sacan las teclas, esta interfaz y su cableado; el render
// no cambia.
class EnemyTestTrigger {
public:
	virtual ~EnemyTestTrigger() = default;

	// Enciende la acción sobre los enemigos (one-shot): la próxima
	// vista los reporta con esa acción y al terminar su ventana
	// vuelven a idle (Death no: es terminal). Una acción a la vez: el
	// mundo simulado decide si la acepta o la ignora (tabla de
	// prioridades). Una acción nueva no necesita un método nuevo.
	virtual void triggerAll(world::ActionKind action) = 0;

	// Proyectiles (hoy el fuego del Skull): avanzan una celda o
	// desaparecen, como lo hará el server cuando mande la posición de
	// cada uno. Solo escriben en el mundo simulado.
	virtual void advanceProjectiles() = 0;
	virtual void despawnProjectiles() = 0;

	// Ataque con un proyectil concreto (hoy el fuego verde de prueba):
	// los enemigos que pueden lanzarlo atacan y disparan ESE proyectil,
	// igual que el ataque normal pero eligiendo el fuego.
	virtual void triggerAttackWith(world::CreatureKind projectile) = 0;
};

}  // namespace game
