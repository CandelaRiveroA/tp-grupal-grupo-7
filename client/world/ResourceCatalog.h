#pragma once

#include <map>
#include <string>

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
// el tamaño en celdas. El tamaño puede variar por estado.
struct ResourceVisual {
	std::string sheetImage;
	int cellWidth = 0;
	int cellHeight = 0;
	int col = 0;
	int row = 0;
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

	bool has(ResourceKind kind, ResourceState state) const;

	const ResourceVisual& visualFor(ResourceKind kind,
					ResourceState state) const;
};

}
