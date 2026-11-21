#pragma once

#include <map>
#include <vector>

#include "game/LocalWorldView.h"
#include "world/CharacterAppearance.h"

namespace game {

// Evento "una entidad empezó una acción": lo emite ActionStartCue en el
// flanco de la acción (una sola vez por inicio). `facing` acompaña para
// que quien reacciona pueda leer los datos de ESA dirección (el ritmo de
// la animación, por ejemplo).
struct ActionStart {
	int id = 0;
	world::CreatureKind kind = world::CreatureKind::Player;
	world::ActionKind action = world::ActionKind::Idle;
	world::Direction facing = world::Direction::Front;
};

// Detecta el FLANCO de "empezó una acción" por entidad: la fuente de
// estado prende entity.action durante la ventana de la acción y la
// apaga al terminar. Devuelve un evento UNA sola vez por inicio (al
// aparecer la acción), para disparar su sonido sin repetir mientras
// dura. Genérico: no conoce qué acción es ni qué suena (eso lo decide
// una tabla en la capa de presentación). Espejo de ToolUseCue, pero por
// entidad y sobre la acción en vez de la herramienta.
class ActionStartCue {
public:
	// Eventos de este tick: una entrada por entidad que ACABA de
	// empezar una acción (las que siguen en la misma acción no emiten).
	std::vector<ActionStart> update(const LocalWorldView& view);

private:
	// Última acción vista por id de entidad (el flanco vive acá).
	std::map<std::uint32_t, world::ActionKind> lastActions_;
};

}  // namespace game
