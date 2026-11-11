#pragma once

#include <map>

#include "world/SpriteVisual.h"

namespace world {

// Estado de la selección de lugar (vocabulario del marcador). Agregar
// uno ("no disponible") = un valor acá + su entrada en buildplace.toml.
enum class BuildPlaceState {
	// Disponible: el piso está trabajado y se puede sembrar.
	Available,
	// Ocupado: la celda ya tiene su cultivo plantado (el cuadrado
	// rojo: "acá ya hay algo").
	Placed,
};

// Bloque de la hoja que arma el cuadrado de un estado: composición
// FIJA de cols x rows celdas (esquinas en las puntas, dos posiciones
// por borde, el interior entremedio) que se dibuja estirada sobre la
// celda seleccionada. sprite.col/row es la esquina sup-izq del bloque.
struct BuildPlaceVisual {
	SpriteVisual sprite;
	int cols = 0;
	int rows = 0;
	// Tamaño sobre la celda seleccionada (1 = la celda entera).
	double scale = 1.0;
};

// Catálogo estado -> bloque. Lo consulta el painter, nunca decide él
// qué estado corresponde (eso es game::PlantingSelection).
class BuildPlaceCatalog {
public:
	void setState(BuildPlaceState state, BuildPlaceVisual visual);

	// nullptr si el estado no tiene bloque cargado.
	const BuildPlaceVisual* find(BuildPlaceState state) const;

private:
	std::map<BuildPlaceState, BuildPlaceVisual> states_;
};

}
