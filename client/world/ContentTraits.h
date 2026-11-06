#pragma once

#include <map>
#include <string>

namespace world {

// Rasgos de un contenido del inventario (semántica de juego, no
// apariencia). Vive APARTE de ContentCatalog a propósito: el catálogo
// describe cómo se ve un contenido, esta tabla qué se puede hacer con
// él. Mañana la regla real la valida el servidor; este rasgo es el
// dato local que hoy usa la pose de "sostener" (game::CarrySelection).
struct ContentTrait {
	bool droppable = false;
};

// Tabla contentId -> rasgos. UNA sola búsqueda por contenido, sin
// switch ni funciones libres (convención §2/§4). Los que no declaran
// rasgos no tienen entrada: isDroppable() sobre un desconocido es
// false (solo lo declarado es soltable).
class ContentTraits {
public:
	// Registra los rasgos de un contenido (pisando los previos).
	void set(const std::string& contentId, ContentTrait trait);

	// nullptr si el contenido no declara rasgos.
	const ContentTrait* find(const std::string& contentId) const;

	bool isDroppable(const std::string& contentId) const;

private:
	std::map<std::string, ContentTrait> traits_;
};

}  // namespace world
