#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "world/Grid.h"
#include "world/NpcVisuals.h"

namespace tomlcfg {
class TomlDocument;
}

namespace world {

// Celda donde la fuente de estado (el servidor) ubica a cada NPC, por id.
// Un NPC que no está acá usa la celda por defecto de su npcs.toml.
using NpcCells = std::map<std::string, GridCell>;

struct NpcDef {
	std::string id;
	std::string room;
	NpcBody body;
	NpcPortrait portrait;
	std::string text;
	NpcPopupStyle popup;

	// Los NPCs sin texto son objetos (p. ej. el cofre): no abren popup.
	bool hasPopup() const;

	// Grilla de slots que muestra al abrirse (`[npcs.container]` del
	// TOML). Sin ella (0×0) no es un contenedor.
	GridSpan container{0, 0};

	// ¿Es un contenedor (cofre) que muestra una grilla de slots?
	bool isContainer() const;

	// Mueble por piezas (`[npcs.furniture]`): `anchor` = a qué celda del
	// sprite se refiere la celda que manda la fuente, en celdas desde la
	// base (abajo-izquierda): x hacia la derecha, y hacia arriba. Default
	// {0, 0} = la base (como cualquier mueble).
	GridCell furnitureAnchor{0, 0};
	// Si el jugador se "mete" en el mueble (la cama).
	std::optional<PropEnter> furnitureEnter;

	// Celda donde está parado: la que mandó la fuente de estado para este
	// id, o la `at` por defecto del TOML si no mandó ninguna.
	GridCell cellIn(const NpcCells& placed) const;
};

// NPCs estáticos del cliente. Se cargan de npcs.toml en el constructor
// (mismo patrón que FontCatalog): datos + geometría pura, sin SDL.
class NpcCatalog {
public:
	explicit NpcCatalog(const std::string& path);

	std::vector<const NpcDef*> forRoom(const std::string& room) const;

	// El NPC con este id (nullptr si no existe). Sirve también para las
	// entradas que solo aportan el contenido de un popup (su `room` no
	// coincide con ninguna escena y nunca se dibujan).
	const NpcDef* find(const std::string& id) const;

	// Botón de cerrar compartido por los popups de contenedor
	// (`[container_close]` de npcs.toml).
	const CloseButton& closeButton() const;

	// NPC bajo un punto de mundo (el último del archivo gana, igual que
	// el resto de los picks). nullptr si no hay ninguno. `placed` son las
	// celdas que mandó la fuente de estado (las mismas con que se dibuja).
	const NpcDef* npcAt(const std::string& room, const PixelPoint& point,
			    int cellPixelSize, const NpcCells& placed) const;

private:
	void readStyles(const tomlcfg::TomlDocument& doc);
	void readCloseButton(const tomlcfg::TomlDocument& doc);
	void readFurniture(const tomlcfg::TomlDocument& doc,
			   const std::string& base, NpcDef& npc) const;
	NpcPopupStyle readStyle(const tomlcfg::TomlDocument& doc,
				const std::string& name) const;
	std::vector<GridCell> readCells(const tomlcfg::TomlDocument& doc,
					const std::string& path) const;
	std::vector<std::vector<GridCell>> readFrames(
		const tomlcfg::TomlDocument& doc, const std::string& path) const;
	NpcDef readNpc(const tomlcfg::TomlDocument& doc,
		       std::size_t index) const;

	std::map<std::string, NpcPopupStyle> styles_;
	CloseButton closeButton_;
	std::map<std::string, Direction> directions_;
	std::vector<NpcDef> npcs_;
};

}
