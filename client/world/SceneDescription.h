#pragma once

#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "world/Color.h"
#include "world/Grid.h"
#include "world/ViewDirection.h"

namespace world {


struct TileSetDef {
	std::string image;
	int cellWidth;
	int cellHeight;
};

enum class DrawMode {
	Single,
	Tile,
	HorizontalStrip,
};

struct DrawSpec {
	DrawMode mode;
	int scaleX;
	int scaleY;
};

// Entrada "entrable" de un elemento (presentación local): el jugador
// puede meterse con el click del cursor. Al hacerlo se ubica en `lie`
// mirando `facing`, y las filas indicadas de la pieza se dibujan ENCIMA
// de las entidades (lo "tapan": en la cama, las frazadas). Es un
// efecto visual del cliente: la fuente de estado no participa.
struct ElementEnter {
	// Filas de la pieza (desde 0, inclusive) que van adelante.
	int firstFrontRow;
	int lastFrontRow;
	// Celda lógica donde se ubica el jugador al meterse, y su pose.
	GridCell lie;
	Direction facing;
};

// Momento del día. Hoy lo setean las teclas de prueba T, C y N; cuando
// el server mande la hora real, game::DayPhase la deriva. Dusk (6 pm) y
// Evening (8 pm) son etapas intermedias de LUZ: para el filtrado por
// `when` del TOML solo existen Day y Night (DayPhase::phase() traduce
// cada etapa a uno de los dos), así que no están en el diccionario del
// reader. Sumar una etapa es: un valor acá + sus filas en las tablas
// de DayPhase: el dibujo no se toca.
enum class TimeOfDay {
	Day,
	Dusk,
	Evening,
	Night,
};

struct ElementPlacement {
	std::string tileset;
	// La pieza que se repite: su tamaño en celdas de hoja y la celda de
	// hoja de cada posición de la pieza (fila por fila). El TOML la
	// describe como bloque contiguo (`region = [col,row,cols,rows]`) o
	// como composición explícita (`piece = [cols,rows]` + `cells`);
	// el reader normaliza ambas formas a esta: un solo camino de
	// dibujo, escalable a cualquier pieza de cualquier hoja.
	GridSpan pieceSize;
	std::vector<GridCell> pieceCells;
	DrawSpec draw;
	GridCell at;
	GridSpan span;
	// "Entrar" al elemento (OPCIONAL, subtabla `enter` del TOML). Sin
	// esto el elemento es decoración pura y nada cambia.
	std::optional<ElementEnter> enter = std::nullopt;
	// Momento del día en el que se dibuja (OPCIONAL, key `when` del
	// TOML: "day" o "night"). Sin la key el elemento se dibuja en
	// cualquier momento; con ella, solo en el suyo (p. ej. la ventana
	// de día y la de noche ocupan la misma celda). El filtro lo
	// aplica el render con visibleAt; el índice espacial no cambia.
	std::optional<TimeOfDay> when = std::nullopt;
	// ¿Se ordena en profundidad con las entidades (el jugador pasa por
	// detrás si está más arriba que su base)? Es para lo que se ve "de
	// pie" sobre el piso: árboles, edificios, muebles. Por defecto no:
	// el fondo y los pisos van siempre debajo de todo (el caso de
	// scenes.toml).
	bool depthSorted = false;

	// ¿Se dibuja en el momento dado? Sin `when`, siempre: el
	// elemento es decoración de cualquier momento del día.
	bool visibleAt(TimeOfDay time) const;
};

// Poblaciones que admite una sala: DATOS puros (sin SDL ni lógica). La
// fuente de estado (el mock hoy, el servidor mañana) lee esto de la
// escena activa y solo puebla lo que la sala permite. El dibujo no
// depende de esto: así cambiar qué sala está activa no acopla nada.
struct SceneSpawns {
	bool animals = false;
	bool minerals = false;
};

// Punto de colocación que dice el mapa (marker del editor): qué es
// (el texto del vocabulario: "player_spawn", "bed", "merchant_pierre")
// y en qué celda. DATOS: qué hacer con cada kind lo decide quien lo usa.
struct SceneMarker {
	std::string kind;
	GridCell cell;
};

// Paso a otro mapa (portal del editor): kind del vocabulario, celda
// donde está y a qué mapa y celda lleva.
struct ScenePortal {
	std::string kind;
	GridCell cell;
	std::string targetMap;
	GridCell targetCell;
};

// Qué pasa al tocar una estructura (edificio del editor). Es vocabulario
// del cliente; el destino lo dice el overlay.
enum class StructureAction {
	None,
	// Abre el popup de un NPC de npcs.toml (el contenido del popup).
	Popup,
	// Pide pasar a otro mapa (`target` = id del mapa).
	EnterMap,
};

// Edificio que colocó el editor (cabaña, mercado, centro comunitario):
// su rol del vocabulario, el rectángulo de celdas que ocupa, la puerta y
// qué elemento de la escena lo dibuja (su caja de click es la del dibujo).
struct SceneStructure {
	std::string role;
	GridCell at;
	GridSpan span;
	std::optional<GridCell> door;
	std::size_t elementIndex = 0;
	StructureAction action = StructureAction::None;
	std::string target;
};

// Una celda de una capa de piso: qué hoja (índice en `TileLayer::
// tilesets`, -1 = vacía) y qué celda de esa hoja se dibuja en ella.
struct LayerCell {
	int tilesetIndex = -1;
	GridCell sheetCell{0, 0};
};

// Capa de tiles en grilla: UNA celda de arte por celda lógica del mapa
// (el piso y los caminos del editor). `cells` es row-major, de tamaño
// size.columns * size.rows. A diferencia de los elementos, no se indexa
// por chunk: el dibujo recorre solo el rango visible (TileLayerLayout).
struct TileLayer {
	GridSpan size{0, 0};
	std::vector<std::string> tilesets;
	std::vector<LayerCell> cells;
};

struct SceneDescription {
	Color floorColor;
	GridSpan grid;
	int cellPixelSize;
	std::unordered_map<std::string, TileSetDef> tilesets;
	// Capas de piso, de abajo hacia arriba: se dibujan antes que los
	// elementos. Vacío = la escena no usa piso en grilla (scenes.toml).
	std::vector<TileLayer> layers;
	std::vector<ElementPlacement> elements;
	std::vector<std::string> soundCues;
	SceneSpawns spawns;
	std::vector<SceneMarker> markers;
	std::vector<ScenePortal> portals;
	std::vector<SceneStructure> structures;
	// Celda por defecto de cada NPC (por id de npcs.toml) que sale de
	// los markers del mapa. La fuente de estado la pisa con la suya.
	std::map<std::string, GridCell> npcCells;
};

}
