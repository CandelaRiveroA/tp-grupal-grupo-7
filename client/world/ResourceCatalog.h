#pragma once

#include <map>
#include <string>

#include "world/SpriteVisual.h"

namespace world {

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

// Recursos picables del mundo indexados por contentId (clave estable del
// contenido, vocabulario compartido con el servidor: la clave de cada
// sección [resources.*] de resources.toml). Vocabulario ABIERTO por
// datos: agregar un recurso = una sección en el TOML, sin tocar código.
struct ResourceCatalog {
	// Ajuste vertical global (px) para apoyar los recursos al piso.
	int offsetY = 0;

	// Escala global del sprite: >1 agranda para tapar el aire transparente
	// de arriba y acercar el recurso a la altura del golpe.
	double scale = 1.0;

	std::map<std::string, std::map<ResourceState, ResourceVisual>> visuals;

	// Una sola búsqueda: nullptr si el par (contentId, state) no tiene
	// visual (p. ej. un id sin sección en el TOML: no se dibuja).
	const ResourceVisual* find(const std::string& contentId,
				   ResourceState state) const;
};

}
