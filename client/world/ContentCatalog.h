#pragma once

#include <map>
#include <string>

#include "world/CropQuality.h"
#include "world/SpriteVisual.h"

namespace world {

// Catálogo GENÉRICO del contenido del inventario: mapea un contentId
// (clave string estable, vocabulario compartido con el servidor) a su
// APARIENCIA (SpriteVisual). El inventario guarda cualquier cosa
// (herramientas, minerales, mañana huevos) sin conocer su tipo concreto:
// solo resuelve contentId -> asset. Reutiliza SpriteVisual (el sprite
// común) como descripción de apariencia, pero NO depende del enum ItemKind.
//
// Lo llena world::InventoryTomlReader desde las subtablas [items.<nombre>]
// de inventory.toml; sdl::InventoryHud lo consume. Agregar contenido es
// editar inventory.toml; no hace falta recompilar.
struct ContentCatalog {
	std::map<std::string, SpriteVisual> visuals;

	// Una sola búsqueda: nullptr si el contentId no tiene visual (no se
	// dibuja). Reemplaza al par has()/visualFor().
	const SpriteVisual* find(const std::string& contentId) const;

	// Búsqueda con calidad: si quality > Base, intenta "contentId:sufijo"
	// (p.ej. "strawberry:good"); si no está registrado, cae a contentId
	// plano. Para Base equivale a find(contentId).
	const SpriteVisual* findQualified(const std::string& contentId,
					  CropQuality quality) const;
};

}  // namespace world
