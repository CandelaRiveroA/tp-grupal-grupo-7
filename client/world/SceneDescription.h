#pragma once

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
};

// Poblaciones que admite una sala: DATOS puros (sin SDL ni lógica). La
// fuente de estado (el mock hoy, el servidor mañana) lee esto de la
// escena activa y solo puebla lo que la sala permite. El dibujo no
// depende de esto: así cambiar qué sala está activa no acopla nada.
struct SceneSpawns {
	bool animals = false;
	bool minerals = false;
};

struct SceneDescription {
	Color floorColor;
	GridSpan grid;
	int cellPixelSize;
	std::unordered_map<std::string, TileSetDef> tilesets;
	std::vector<ElementPlacement> elements;
	std::vector<std::string> soundCues;
	SceneSpawns spawns;
};

}
