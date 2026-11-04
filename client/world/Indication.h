#pragma once

namespace world {

// Lo que una entidad le "dice" al jugador (un diálogo sobre ella). Es
// ESTADO DE JUEGO: lo manda la fuente (el server; hoy el mock). Agregar
// una indicación = un valor acá + su diálogo en ui/dialogs.toml; el
// dibujo no cambia.
enum class Indication {
	ReadyToMilk,
};

}
