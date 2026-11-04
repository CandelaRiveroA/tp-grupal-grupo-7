#pragma once

#include <map>
#include <string>

#include "world/SpriteVisual.h"

namespace world {

// Recurso picable del mundo (vocabulario del juego, lo comunica el
// servidor). Agregar uno = un valor acá + sus datos en el TOML.
enum class ResourceKind {
	Silver,
	Coal,
	Lapis,
	Gold,
};

// Estado de daño que decide el servidor. Son los estados VISIBLES: al
// agotarse, el recurso sale de la vista (no es un estado dibujable).
enum class ResourceState {
	Intact,
	Cracked,
	Broken,
};

// Bloque de la hoja a dibujar: col/row es la esquina sup-izq; cols/rows
// el tamaño en celdas. El tamaño puede variar por estado. Compone el
// sprite común (sus campos drawn no aplican a bloques de celdas).
struct ResourceVisual {
	SpriteVisual sprite;
	int cols = 1;
	int rows = 1;
};

struct ResourceCatalog {
	// Ajuste vertical global (px) para apoyar los recursos al piso.
	int offsetY = 0;

	// Escala global del sprite: >1 agranda para tapar el aire transparente
	// de arriba y acercar el recurso a la altura del golpe.
	double scale = 1.0;

	std::map<ResourceKind, std::map<ResourceState, ResourceVisual>> visuals;

	// Una sola búsqueda: nullptr si el par (kind, state) no tiene visual.
	const ResourceVisual* find(ResourceKind kind, ResourceState state) const;
};

}
