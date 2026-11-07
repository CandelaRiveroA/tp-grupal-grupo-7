#pragma once

namespace game {

// ANDAMIAJE TEMPORAL (solo cliente, borrable de una pieza). Disparador
// de prueba para encender la acción de los enemigos con el teclado,
// mientras el server todavía no manda esas acciones. El seam real es
// game::ViewEntity::action (contrato §6/§10): el render ya anima
// cualquier acción que llegue en la vista. Esto SOLO escribe en esa
// misma fuente (hoy el "server falso"), para probar la animación.
//
// Interfaz chica y enfocada (§9): PlaySession depende de esto, no del
// tipo concreto del mundo simulado. El día que el server mande las
// acciones, se saca la tecla, esta interfaz y su cableado; el render no
// cambia.
class EnemyTestTrigger {
public:
	virtual ~EnemyTestTrigger() = default;

	// Enciende el ataque de los enemigos (one-shot): la próxima vista
	// los reporta con ActionKind::Attack; al terminar su ventana
	// vuelven a caminar/idle.
	virtual void attackAll() = 0;
};

}  // namespace game
