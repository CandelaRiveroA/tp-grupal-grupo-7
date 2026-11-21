#pragma once

#include <map>
#include <string>
#include <vector>

#include "sdl/render/SheetCache.h"
#include "world/NpcCatalog.h"

namespace sdl {
class SdlContext;
class Viewport;
}

namespace sdl {

// Dibuja los cuerpos estáticos de los NPCs de la sala actual.
class NpcRenderer {
public:
	explicit NpcRenderer(SdlContext& context);

	void setNpcs(const std::vector<const world::NpcDef*>& npcs);
	// `placed`: celdas que mandó la fuente de estado (por id); el resto
	// queda en su celda por defecto.
	// Cuadro actual de cada objeto animado, por id (el que no está, en 0).
	void setFrames(const std::map<std::string, int>& frames);
	void draw(const Viewport& viewport, int cellPixelSize,
		  const world::NpcCells& placed);

private:
	SheetCache sheets_;
	std::vector<const world::NpcDef*> npcs_;
	std::map<std::string, int> frames_;
};

}
