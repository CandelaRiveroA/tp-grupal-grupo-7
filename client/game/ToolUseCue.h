#pragma once

#include <optional>

#include "world/ItemKind.h"

namespace game {

// Detecta el FLANCO de "empezó a usarse una herramienta": la fuente de
// estado prende activeTool durante la ventana de la acción y lo apaga
// al terminar. Devuelve la herramienta UNA sola vez por uso (al
// aparecer), para disparar su sonido sin repetir mientras dura la
// acción. Genérico: no conoce qué herramienta es ni qué suena (eso lo
// decide una tabla en la capa de presentación). Espejo de FootstepCue.
class ToolUseCue {
public:
	// activeTool: la herramienta en uso este tick (nullopt si ninguna).
	// Devuelve la herramienta solo en el flanco nullopt -> tool; en los
	// ticks siguientes (misma herramienta sostenida) devuelve nullopt.
	std::optional<world::ItemKind> update(
		const std::optional<world::ItemKind>& activeTool);

private:
	bool using_ = false;
};

}  // namespace game
